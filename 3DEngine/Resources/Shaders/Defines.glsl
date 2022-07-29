

vec2 sign_not_zero(vec2 v) 
{
        return fma(step(vec2(0.0), v), vec2(2.0), vec2(-1.0));
}