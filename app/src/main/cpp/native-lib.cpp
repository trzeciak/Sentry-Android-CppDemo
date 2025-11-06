#include <string>

#include <execinfo.h>

#include <android/log.h>
#include <jni.h>

#include <sentry.h>

#include "native-lib.hpp"

#define LOG_TAG "CppDemo"
#define LOG_PRINT(...) do { \
    char buffer[1024]; \
    snprintf(buffer, sizeof(buffer), __VA_ARGS__); \
    cppdemo::NativeLogger(buffer);             \
} while(0)

static bool NativeInitialized = false;

extern "C" JNIEXPORT void JNICALL
Java_com_example_cppdemo_MainActivity_nativeInitialization(JNIEnv* env, jobject /* this */) {
    cppdemo::NativeInitialization();
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_cppdemo_MainActivity_stringHello(JNIEnv* env, jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_cppdemo_MainActivity_printGreetings(JNIEnv *env, jobject /* this */) {
    LOG_PRINT("Greetings from C++!");
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_cppdemo_MainActivity_callAbort(JNIEnv *env, jobject /* this */) {
    cppdemo::CallAbort();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_cppdemo_MainActivity_throwCppException(JNIEnv *env, jobject /* this */) {
    cppdemo::ThrowCppException();
}

#pragma mark -
#pragma mark Cpp Demo Implementation

inline void cppdemo::NativeLogger(const char *message) {
    __android_log_print(ANDROID_LOG_WARN, LOG_TAG, "%s", message);
    sentry_value_t crumb = sentry_value_new_breadcrumb("debug", message); // type: debug / default, ... https://develop.sentry.dev/sdk/data-model/event-payloads/breadcrumbs/#breadcrumb-types
    sentry_value_set_by_key(crumb, "category", sentry_value_new_string("CppDemo"));
    sentry_value_set_by_key(crumb, "level", sentry_value_new_string("debug")); // fatal, error, warning, info, debug
    //if (timestamp) {
    //    sentry_value_set_by_key(crumb, "timestamp", sentry_value_new_string(timestamp));
    //}
    sentry_add_breadcrumb(crumb);
}

void cppdemo::NativeInitialization() {

    if (NativeInitialized) {
        LOG_PRINT("Native already initialized");
        return;
    }

    NativeInitialized = true;

    LOG_PRINT("Initializing native code");

    static std::terminate_handler PreviousTerminateHandler = std::set_terminate([] {
        LOG_PRINT("Custom terminate lambda handler (!)");

        sentry_value_t event = sentry_value_new_message_event(
            SENTRY_LEVEL_DEBUG,
            nullptr,
            "C++ unhandled exception (from terminate handler)"
        );

        constexpr size_t MaxFrames = 64;

        void *frames[MaxFrames];
        int framesCount = backtrace(frames, MaxFrames);
        if (framesCount > 2) {
            // override throw backtrace frame, on the exception (probably) thrown, for readability reason
            std::swap(frames[0], frames[framesCount-2]);
        }

        sentry_value_t stacktrace = sentry_value_new_stacktrace(frames, framesCount);
        LOG_PRINT("  .stacktrace:\n%s", sentry_value_to_json(stacktrace));

        const char *exception_type = "Unknown";
        const char *exception_message = "No exception information available";

        if (std::exception_ptr eptr = std::current_exception()) {
            try {
                std::rethrow_exception(eptr);
            }
            catch (const std::exception &e) {
                exception_type = typeid(e).name();
                exception_message = e.what();
            }
            catch (...) {
                exception_type = "Unknown C++ Exception";
                exception_message = "Non-std::exception caught";
            }
        }

        sentry_value_t exception = sentry_value_new_exception(exception_type, exception_message);

        sentry_value_set_by_key(exception, "stacktrace", stacktrace);

        sentry_value_t mechanism = sentry_value_new_object();
        sentry_value_set_by_key(mechanism, "type", sentry_value_new_string("cpp_terminate"));
        sentry_value_set_by_key(mechanism, "handled", sentry_value_new_bool(true));

        sentry_value_t data = sentry_value_new_object();
        if (framesCount > 2) {
            sentry_value_set_by_key(data, "dev_info", sentry_value_new_string("first frame was swapped with backtrace, to improve grouping events"));
        }
        sentry_value_set_by_key(mechanism, "data", data);
        sentry_value_set_by_key(exception, "mechanism", mechanism);

        sentry_event_add_exception(event, exception);

        LOG_PRINT("  .exception-body:\n%s", sentry_value_to_json(event));

        sentry_uuid_t uuid = sentry_capture_event(event);
        char eventID[37];
        sentry_uuid_as_string(&uuid, eventID);

        LOG_PRINT("  .corresponding-unhandled-exception-event-uuid: %s", eventID);

        sentry_flush(2000); // [ms]

        PreviousTerminateHandler();
    });
}

void cppdemo::CallAbort() {
    InternalCallAbort();
}

void cppdemo::InternalCallAbort() {
    LOG_PRINT("Before call abort()");
    abort();
}

void cppdemo::ThrowCppException() {
    ThrowCppExceptionDeepOne();
}

void cppdemo::ThrowCppExceptionDeepOne() {
    ThrowCppExceptionDeepTwo();
}

void cppdemo::ThrowCppExceptionDeepTwo() {
    ThrowCppExceptionDeepThree();
}

void cppdemo::ThrowCppExceptionDeepThree() {
    InternalThrowCppException();
}

void cppdemo::InternalThrowCppException() {
    LOG_PRINT("Before throw a C++ exception");
    throw std::runtime_error("Uncaught C++ exception!");
}
