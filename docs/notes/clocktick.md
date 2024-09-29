# Megha Operating System V2
------------------------------------------------------------------------------

##  Basic time keeping in MeghaOS
categories: note, independent
19 Sept 2024

The osdev wiki in the example mentions that if interrupt is triggered after every `1.75ms` and we
keep the elapsed time as integer (truncating the fraction) there will be a huge error. That is true,
but my argument here is that if we keep time not in millisecond but in microsecond the effect of the
loss of fractional part can be within acceptable range. Taking the above example, but using
microseconds instead of milliseconds, after one interrupt we will say `1000us` has passed but
actually `1000.75us` would have passed. Now there is still the same deviation of `0.75`, but because
working with larger numbers it is now `0.75us` not `0.75ms`. This is similar to the effect of taking
a mug of water from a bucket compared to a pond. Now lets generalise the effect of using integer to
store a number which here is time.

Lets say `T` is the actual time period of the interrupt clock and `t` is the time we are keeping.
This `t` is equal to `T` but fractional part removed. Note that both `T` and `t` are constants, they
do not change over time. Now lets do some calculation to see what is the effect of keeping time in
integer.

In the worst case `T` have the form `x.9999..` making `t` = `floor(T) = x`. Therefore the maximum
difference `T - t` will be `0.999..`, for convenience lets say `T - t = 1.00`. Please note that this is
the worst case.

```
When
    T - t = 1
=>  T = t + 1

```

After `N` interrupts we would have tracked `N` units of time less. In other words our measure of
time would lag, that is more time would actually pass when we call delay.

```
Actual time elapsed = T * N
                   => (t + 1) * N
Tracked time        = t * N

Deviation between actual and tracked time after `N` interrupts
                    = (t + 1) * N - (t * N)
                   => N

Rate of deviation over N interrupts
                    = difference / actual time
                   => N / (T * N)
                   => 1/T
                   => 1/(t + 1)

Percent deviation over N interrupts
                    = 100 * Rate of deviiation
                   => 100/(t + 1)

```

This means that tracking time using an integer would cause an inaccuracy of `100/(t + 1)` percent.
Lets use the calculation to explain how `unit of time` effects the inaccuracy as mentioned
at the start of this document. Note that is the calculations are from the worst case perspective
when `T` is of the form `x.999..`.

| unit of time | N        | t      | T         | Actual time | Tracked time | Difference | %    |
|              |          |        | = (t + 1) | = (T * N)   | = (t * N)    |            |      |
|--------------|----------|--------|-----------|-------------|--------------|------------|------|
| milliseconds | 1000     | 1ms    | 2ms       | 2000ms      | 1000ms       | 1000ms     | 100% |
| microseconds | 1000     | 1000us | 1001us    | 1001ms      | 1000ms       | 1ms        | 0.1% |
| microseconds | 1000,000 | 1000us | 1001us    | 1001,000ms  | 1000,000ms   | 1000ms     | 0.1% |

As you can see just by working with lower unit of time (thus working with larger numbers) the effect
precision loss can be reduced to insignificant levels from `1000ms` to `1ms`. Also note from the
above table, the number of interrupts required to reach the same deviation in tracked time when
using microseconds instead of milliseconds - `1000,000` interrupts instead of `1000` to reach
`1000ms` for the chosen `t`. This is what the above `Rate of deviation` indicates.

------------------------------------------------------------------------------

##  Interrupt clock frequency and tick period considerations
categories: note, independent
19 Sept 2024

The clock interrupt frequency should be so that we can do everything we are using the interrupt for.
If we run the clock too fast there will be too many wasteful interrupt calls and if too slow we
could not cater to the requirements.

Elaborating the above point, say if tick period is 1000us, but we are interrupting every 50us, then
there will be 20 interrupt calls just to increment the tick count by one at the end of 20th
interrupt call, that is the use. This seems wasteful to interrupt every 50us just for the tick
count. So there is nothing else to do in the timer interrupt or if everything thing is based on the
tick period, then it is ideal to make the tick period and timer period the same.

For now I think tick period and thus interrupt timer period should be `1000us`, which means
a `Percent deviation` of `0.1%` which is good enough. The `0.1%` deviation means that over 10 mins of
continous counting there will be a deviation of `0.6 seconds`, that is acceptable to me.
