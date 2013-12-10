#include <node.h>
#include <v8.h>

#include <windows.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <string>

#include "fix.h"

using namespace v8;
using namespace std;
int pows[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
                25,26,27,28,29,30,31,32};
HINSTANCE fixtools;
HINSTANCE vdba;

// this helps to extract the bits stored in the DWORD
int sorted_pos[] = {24, 25, 26, 27, 28, 29, 30, 31,
                  16, 17, 18, 19, 20, 21, 22, 23,
                  8, 9, 10, 11, 12, 13, 14, 15,
                  0, 1, 2, 3, 4, 5, 6, 7};

typedef int (__stdcall * pICFUNC)(char *, char*, char*, float* );
pICFUNC get_float;
typedef int (__stdcall * pICFUNC3)(char *, char*, char*, double* );
pICFUNC3 get_double;
typedef int (__stdcall * pICFUNC2)(char *node, char *tag, char *field, void *b);
pICFUNC2 get_binary;


/** FIXTOOLS functions **/
int ft_FixLogin(char* usr, char* pwd) {

   /* get pointer to the function in the dll*/
   FARPROC lpfnGetProcessID = GetProcAddress(HMODULE (fixtools), 
                                                "FixLogin@8");
  
   /*
      Define the function in the DLL for reuse. 
      This is just prototyping the dll's function.
      A mock of it. Use "stdcall" for maximum compatibility.
   */
   typedef int (__stdcall * pICFUNC)(char *, char*);
   float* val = (float*)malloc(sizeof(float)*1);
   pICFUNC MyFunction = pICFUNC(lpfnGetProcessID);
   /* The actual call to the function contained in the dll */
   int intMyReturnVal = MyFunction(usr, pwd);
   /* The return val from the dll */
    return intMyReturnVal;
}

Handle<Value> FixLogin(const Arguments& args) {
    HandleScope scope;
    int result;
    char username[20];
    char password[20];

    // get the params
    v8::String::Utf8Value param0(args[0]->ToString()); // username
    v8::String::Utf8Value param1(args[1]->ToString()); // password
    // convert them to string
    std::string s_param0 = std::string(*param0);    
    std::string s_param1 = std::string(*param1);    
    strcpy(username, s_param0.c_str());
    strcpy(password, s_param1.c_str());
   
    result = ft_FixLogin(username, password);

    // return the sorted 64-bit string representing a vial
    return scope.Close(Integer::New(result));
}


int ft_FixLogout(){
    FARPROC lpfnGetProcessID = GetProcAddress(HMODULE (fixtools),
                                                "FixLogout@0");
    typedef int (__stdcall *pICFUNC)();
    pICFUNC MyFunction = pICFUNC(lpfnGetProcessID);
    int intMyReturnVal = MyFunction();
    return intMyReturnVal;
}

Handle<Value> FixLogout(const Arguments& args) {
    HandleScope scope;
    int result;
    result = ft_FixLogout();
    return scope.Close(Integer::New(result));
}


int ft_FixGetCurrentUser(char* username, char* fullname, char* groupname) {
    
    FARPROC lpfnGetProcessID = GetProcAddress(HMODULE (fixtools), 
                                                "FixGetCurrentUser@24");
    typedef int (__stdcall * pICFUNC)(char *, int, char *, int, char *, int);
    pICFUNC MyFunction = pICFUNC(lpfnGetProcessID);
    int intMyReturnVal = MyFunction(username, LOGIN_NAMESIZE, fullname,
                                    NAMESIZE, groupname, GROUP_NAMESIZE);
    return intMyReturnVal;
}


Handle<Value> FixGetCurrentUser(const Arguments& args) {
    HandleScope scope;
    char username[8];
    char fullname[32];
    char groupname[32];

    ft_FixGetCurrentUser(username, fullname, groupname);

    // build dict to return  
    // [username: 'user1', fullname: 'John Doe', group: 'OPERATORS']
    Local<Array> result = Array::New();
    result->Set(String::New("username"), String::New(username));
    result->Set(String::New("fullname"), String::New(fullname));
    result->Set(String::New("group"), String::New(groupname));
    return scope.Close(result);
}

/** VBDA functions **/
pICFUNC2 eda_get_one_binary() {
    FARPROC lpfnGetProcessID = GetProcAddress(HMODULE (vdba),"eda_get_one_binary");
    typedef int (__stdcall * pICFUNC2)(char *node, char *tag, char *field, void *b);
   return pICFUNC2(lpfnGetProcessID);

}

pICFUNC eda_get_one_float() {
   FARPROC lpfnGetProcessID = GetProcAddress(HMODULE (vdba),"eda_get_one_float");
   typedef int (__stdcall * pICFUNC)(char *, char*, char*, float* );
   return pICFUNC(lpfnGetProcessID);
}

pICFUNC3 eda_get_one_double() {
   FARPROC lpfnGetProcessID = GetProcAddress(HMODULE (vdba),"eda_get_one_double");
   typedef int (__stdcall * pICFUNC3)(char *, char*, char*, double* );
   return pICFUNC3(lpfnGetProcessID);
}

int get_32_bits(char* node, char* tag, char* res) {
    double val ;

    get_double(node, tag, "E_CV", &val);
    for (int i = 0; i < 32; i++)
        res[sorted_pos[i]] = ((int) val & pows[i]) ? '1' : '0';
        
    return 0;
}




Handle<Value> GetFloat(const Arguments& args) {
    HandleScope scope;
    float res;
    
    char node[20];
    char tag[41];
    // get the params
    v8::String::Utf8Value param0(args[0]->ToString()); // node name
    v8::String::Utf8Value param1(args[1]->ToString()); // tag name
    // convert them to string
    std::string s_param0 = std::string(*param0);    
    std::string s_param1 = std::string(*param1);    
    strcpy(node, s_param0.c_str());
    strcpy(tag, s_param1.c_str());
    
    get_float(node, tag, "F_CV", &res);

    // return the sorted 64-bit string representing a vial
    return scope.Close(Number::New(res));
}

int get_vial_bits(char* node, char* tag, char* res) {
    char str[41];
    double val;

    res[64] = '\0';

    strcpy (str, tag);
    strcat (str,"_0\0");
    get_double(node, str, "E_CV", &val);
    for (int i = 0; i < 32; i++) 
        *(res + sorted_pos[i]) = 48+(((int) val & pows[i]) != 0 );
    


    strcpy (str, tag);
    strcat (str,"_1\0");
    get_double(node, str, "E_CV", &val);
    for (int i = 0; i < 32; i++)
        *(res + sorted_pos[i] + 32) = 48+(((int) val & pows[i]) != 0 );

    return 0;
}


Handle<Value> GetVialColumn(const Arguments& args) {
    HandleScope scope;
    
    char res[65];
    char node[20];
    char tag[41];
    // get the params
    v8::String::Utf8Value param0(args[0]->ToString()); // node name
    v8::String::Utf8Value param1(args[1]->ToString()); // tag name
    // convert them to string
    std::string s_param0 = std::string(*param0);    
    std::string s_param1 = std::string(*param1);    
    strcpy(node, s_param0.c_str());
    strcpy(tag, s_param1.c_str());
   
    get_vial_bits(node, tag, res);
    
    // return the sorted 64-bit string representing a vial
    return scope.Close(String::New(res));
}




void init(Handle<Object> exports) {
    // Get handles of the 2 libraries
    // Get vdba handle
    vdba = LoadLibrary("C:\\Program Files (x86)\\Proficy\\Proficy iFIX\\vdba.dll");
    // Get fixtools handle
    fixtools =  LoadLibrary("C:\\Program Files (x86)\\Proficy\\Proficy iFIX\\fixtools.dll");

    // Get address of all time-critical functions for quick reference
    // put function in separate part
    get_double = eda_get_one_double();
    get_float = eda_get_one_float();

    //initialize pows of 2
    for (int i=0;i<32;i++)
        pows[i]=(int)pow(2.0, (i));

 
 
    exports->Set(String::NewSymbol("GetVialColumn"),
        FunctionTemplate::New(GetVialColumn)->GetFunction());

    exports->Set(String::NewSymbol("FixLogin"),
        FunctionTemplate::New(FixLogin)->GetFunction());
    exports->Set(String::NewSymbol("FixLogout"),
        FunctionTemplate::New(FixLogout)->GetFunction());
    exports->Set(String::NewSymbol("FixGetCurrentUser"),
        FunctionTemplate::New(FixGetCurrentUser)->GetFunction());

    exports->Set(String::NewSymbol("GetFloat"),
        FunctionTemplate::New(GetFloat)->GetFunction());

    
}

NODE_MODULE(edajs, init);

