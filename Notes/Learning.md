Timers for the ESP32
For the setTimer() function, the first argument is the time in milliseconds, and the second argument is the function to call when the timer expires. The third argument is the type of timer, which can be either once or repeat. If the third argument is not specified, the default is once.
```c
vTimerCallback( TimerHandle_t xTimer )
{
    //Here goes the execution of the code
}
```

6130520241958386978

