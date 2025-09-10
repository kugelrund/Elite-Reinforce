# Movement Key Effect on Air Acceleration

In vanilla Quake 3 you can accelerate basically just as well with a single input key (i.e. only A or only D) than with the usual combination of two keys (W+A or W+D).
But in the Elite Force singleplayer, strafing with only a single key is significantly slower while in the air.
On the other hand, acceleration in Quake 3 is significantly slower when holding the jump key while in the air, whereas it has no influence whatsoever in Elite Force.

## Code in Quake 3

While in the air, `PM_AirMove` determines how the player accelerates.
In Quake 3, this function contains the line ([Quake-III-Arena/code/game/bg_pmove.c#L634](https://github.com/id-Software/Quake-III-Arena/blob/dbe4ddb10315479fc00086f08e25d968b4b43c49/code/game/bg_pmove.c#L634))

```c
wishspeed *= scale;
```

right before calling `PM_Accelerate` with `wishspeed` as argument.
The higher `wishspeed`, the more acceleration can be squeezed out of `PM_Accelerate` by the player when using the optimal viewangle.
So to increase the potential for acceleration, players would want `wishspeed` to be as large as possible.

The value for `scale` is set using the function `PM_CmdScale` ([Quake-III-Arena/code/game/bg_pmove.c#L290](https://github.com/id-Software/Quake-III-Arena/blob/dbe4ddb10315479fc00086f08e25d968b4b43c49/code/game/bg_pmove.c#L290))
and the initial `wishspeed` (before being scaled with `scale`) is set to the length of the `wishvel` vector ([Quake-III-Arena/code/game/bg_pmove.c#L628](https://github.com/id-Software/Quake-III-Arena/blob/dbe4ddb10315479fc00086f08e25d968b4b43c49/code/game/bg_pmove.c#L628)).
But in a nutshell, both `scale` and the initial `wishspeed` only depend on the number of axes for which the player performs an input (assuming uniform running speed).
The possible axes are longitudinal (`+forward` or `+back`), lateral (`+moveleft` or `+moveright`) and vertical (`+moveup` or `+movedown`).

When longitudinal and lateral inputs are both active, the initial `wishspeed` is $`\sqrt{2}`$ times as **large** as when only one of them is active.
The inverse of this applies to `scale`; simultaneous longitudinal and lateral input means that `scale` is $`\sqrt{2}`$ times as **small** than with only one of those inputs.
Since `wishspeed` is multiplied with `scale`, this factor of $`\sqrt{2}`$ cancels out.
That means no matter if both longitudinal and lateral inputs are active or only one of them, `wishspeed` is of the same length, leading to equal potential for acceleration.
This is why the half-beat strafing technique is very viable in Quake 3.

Moreover, `scale` is $`\sqrt{3}`$ times as small when longitudinal, lateral and vertical inputs are all active.
But for `wishspeed` the vertical axis is ignored, so it still only contains a factor of $`\sqrt{2}`$ in this case.
Therefore, holding the jump input while in the air causes the resulting `wishspeed` to be smaller than when not holding jump, which leads to less potential for acceleration.
This is why in Quake 3 DeFRaG it is optimal to only tap jump exactly when hitting the ground, rather than buffering it.

## Code Difference in Elite Force

In Elite Force, the line of code for scaling `wishspeed` with `scale` does not exist ([stvoyef/game/bg_pmove.cpp#L622](https://github.com/kugelrund/Elite-Reinforce/blob/04e0e618d1ee57a2919f1a852a688c03b1aa155d/game/bg_pmove.cpp#L622)).
Therefore the effects of it for Quake 3 described above do not apply to Elite Force.

When longitudinal and lateral inputs are both active, `wishspeed` is $`\sqrt{2}`$ times as **large** as when only one of them is active and there is no multiplication with `scale` that could cancel out this factor.
So there is a larger potential for acceleration when both longitudinal and lateral inputs are active.
This is why half-beat strafing is quite suboptimal in Elite Force and the usual strafing (W+A or W+D) should be preferred.

On the other hand, additional vertical inputs do not influence the resulting `wishspeed` anymore.
So jump inputs do not change the potential for acceleration.
This is why there is no need to exactly time your jump in Elite Force, players can buffer as much as they like without losing acceleration potential.

Funnily enough, `scale` is still computed at the start of `PM_AirMove` ([stvoyef/game/bg_pmove.cpp#L604](https://github.com/kugelrund/Elite-Reinforce/blob/04e0e618d1ee57a2919f1a852a688c03b1aa155d/game/bg_pmove.cpp#L604)), but remains unused in the function.
Also worth noting is that for all other movement functions, Elite Force mirrors Quake 3's `wishspeed` exactly.
For `PM_WalkMove` for example, both Elite Force ([stvoyef/game/bg_pmove.cpp#L717](https://github.com/kugelrund/Elite-Reinforce/blob/04e0e618d1ee57a2919f1a852a688c03b1aa155d/game/bg_pmove.cpp#L717)) and Quake 3 ([Quake-III-Arena/code/game/bg_pmove.c#L750](https://github.com/id-Software/Quake-III-Arena/blob/dbe4ddb10315479fc00086f08e25d968b4b43c49/code/game/bg_pmove.c#L750)) do have the scaling.
So the behaviour for prestrafes on the ground is actually the same and the difference only shows while in the air.

## Air Deceleration

Seemingly related, Elite Force adds the new lines ([stvoyef/game/bg_pmove.cpp#L634](https://github.com/kugelrund/Elite-Reinforce/blob/04e0e618d1ee57a2919f1a852a688c03b1aa155d/game/bg_pmove.cpp#L634))

```c++
if ( ( DotProduct (pm->ps->velocity, wishdir) ) < 0.0f )
{//Encourage deceleration away from the current velocity
    wishspeed *= pm_airDecelRate;
}
```

right were the normal scaling from Quake 3 would have been.
This means `wishspeed` is increased whenever the user input points away from the current movement direction, effectively making it easier to slow down in the air.

That addition really makes it seem like Raven was experimenting with air acceleration here.
Perhaps during this experimentation, the original scaling was removed - be it intentionally or unintentionally.
