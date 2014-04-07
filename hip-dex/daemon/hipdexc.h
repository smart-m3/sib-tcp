#ifndef HIPDEXC_HEADER
#define HIPDEXC_HEADER

#ifdef __cplusplus
extern "C" {
#endif

    int HIPDEX_Start();
    
    int HIPDEX_Stop();
    
    int HIPDEX_Initiate(unsigned char *, char *);
    
    void *HIPDEX_GetSA(long);
    
    void HIPDEX_RemoveSA(long);
    
    void HIPDEX_Encrypt(void*, char*, char*, int);
    
    void HIPDEX_Decrypt(void*, char*, char*, int);

#ifdef __cplusplus
}
#endif

#endif