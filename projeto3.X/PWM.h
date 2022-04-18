
#ifndef PWM_H
#define PWM_H

#ifdef __cplusplus
extern "C" {
#endif

void PWMInit(void);
void setPWM(unsigned int duty);

#ifdef __cplusplus
}
#endif

#endif /* PWM_H */
