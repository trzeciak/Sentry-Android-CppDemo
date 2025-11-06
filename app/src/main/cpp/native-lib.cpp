#include <string>

#include <execinfo.h>

#include <android/log.h>
#include <jni.h>

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

        constexpr size_t MaxFrames = 64;

        void *frames[MaxFrames];
        int framesCount = backtrace(frames, MaxFrames);
        if (framesCount > 2) {
            // override throw backtrace frame, on the exception (probably) thrown, for readability reason
            std::swap(frames[0], frames[framesCount-2]);
        }

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

        LOG_PRINT("  .exception(.type: %s, message: %s)", exception_type, exception_message);

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
