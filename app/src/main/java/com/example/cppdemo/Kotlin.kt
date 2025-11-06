package com.example.cppdemo

class Kotlin {

    companion object {
        @JvmStatic
        fun throwKotlinStyleException() {
            throw IllegalStateException("Intentional Kotlin-style crash from Java!")
        }
    }
}
