#if !defined(__language__)
# error "__language__ not defined"
#elif __language__ < 30000
# error "Why are you using such an old version of Codira?"
#elif __language__ >= 810000
# error "Is Codira 81 out already? If so, please update this test."
#else
void language3ReadyToGo();
#endif
