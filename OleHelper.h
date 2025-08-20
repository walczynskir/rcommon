// OleHelper.h

#ifdef __cplusplus
extern "C" {
#endif


// borrowed from OLE2UI sample library
STDAPI_(void) OleUIMetafilePictIconFree(HGLOBAL);
STDAPI OleStdSwitchDisplayAspect(LPOLEOBJECT,
                                 LPDWORD,
                                 DWORD,
                                 HGLOBAL,
                                 BOOL,
                                 BOOL,
                                 LPADVISESINK,
                                 LPBOOL);


#ifdef __cplusplus
} // extern "C"
#endif
