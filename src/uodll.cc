#include <node.h>
#include "uodll.h"

using namespace v8;
using namespace node;

OPEN Open;
CLOSE Close;
VERSION Version;
PUSHNIL PushNil;
PUSHBOOLEAN PushBoolean;
PUSHINTEGER PushInteger;
PUSHDOUBLE PushDouble;
PUSHSTRREF PushStrRef;
PUSHSTRVAL PushStrVal;
GETBOOLEAN GetBoolean;
GETINTEGER GetInteger;
GETDOUBLE GetDouble;
GETSTRING GetString;
GETTOP GetTop;
GETTYPE GetType;
INSERT Insert;
PUSHVALUE PushValue;
REMOVE Remove;
SETTOP SetTop;
MARK Mark;
CLEAN Clean;
EXECUTE Execute;

Handle<Value> GetHandle(const Arguments& args) {
  HandleScope scope;
  return scope.Close(Integer::New(Open()));
}
Handle<Value> CloseHandle(const Arguments& args) {
  HandleScope scope;
  if (args.Length() < 1) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }
  if (!args[0]->IsNumber()) {
    ThrowException(Exception::TypeError(String::New("Wrong argument")));
    return scope.Close(Undefined());
  }
  Close(args[0]->Int32Value());
  return scope.Close(Undefined());
}

Handle<Value> Call(const Arguments& args) {
  HandleScope scope;
  if (args.Length() < 2) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }
  if (!args[0]->IsNumber()) {
    ThrowException(Exception::TypeError(String::New("Wrong argument")));
    return scope.Close(Undefined());
  }

  Local<Array> params = Local<Array>::Cast(args[1]);
  int handle = args[0]->Int32Value();
  int params_length = params->Length();
  Local<Value> value;

  SetTop(handle, 0);
  for (int i = 0; i < params_length; i++) {
    value = params->Get(i);
    if (value->IsBoolean()) {
      PushBoolean(handle, value->BooleanValue());
    } else if (value->IsNumber()) {
      PushInteger(handle, value->Int32Value());
    } else if (value->IsString()) {
      PushStrVal(handle, *String::Utf8Value(value->ToString()));
    }
  }
  Execute(handle);

  int result_length = GetTop(handle);
  Local<Array> results = Array::New(result_length);

  for (int i = 0; i < result_length; i++) {
    switch(GetType(handle, i+1)) {
      case 0:
        results->Set(i, Null());
        break;
      case 1:
        results->Set(i, Boolean::New(GetBoolean(handle, i+1)));
        break;
      case 3:
        results->Set(i, Integer::New(GetInteger(handle, i+1)));
        break;
      case 4:
        results->Set(i, String::New(GetString(handle, i+1)));
        break;
    }
  }

  return scope.Close(results);
}

void Init(Handle<Object> target) {
  HINSTANCE hDLL = LoadLibrary("uo.dll");
  if (hDLL != NULL) {
    Open = (OPEN)GetProcAddress(hDLL, "Open");
    Close = (CLOSE)GetProcAddress(hDLL, "Close");
    Version = (VERSION)GetProcAddress(hDLL, "Version");
    PushNil = (PUSHNIL)GetProcAddress(hDLL, "PushNil");
    PushBoolean = (PUSHBOOLEAN)GetProcAddress(hDLL, "PushBoolean");
    PushInteger = (PUSHINTEGER)GetProcAddress(hDLL, "PushInteger");
    PushDouble = (PUSHDOUBLE)GetProcAddress(hDLL, "PushDouble");
    PushStrRef = (PUSHSTRREF)GetProcAddress(hDLL, "PushStrRef");
    PushStrVal = (PUSHSTRVAL)GetProcAddress(hDLL, "PushStrVal");
    GetBoolean = (GETBOOLEAN)GetProcAddress(hDLL, "GetBoolean");
    GetInteger = (GETINTEGER)GetProcAddress(hDLL, "GetInteger");
    GetDouble = (GETDOUBLE)GetProcAddress(hDLL, "GetDouble");
    GetString = (GETSTRING)GetProcAddress(hDLL, "GetString");
    GetTop = (GETTOP)GetProcAddress(hDLL, "GetTop");
    GetType = (GETTYPE)GetProcAddress(hDLL, "GetType");
    Insert = (INSERT)GetProcAddress(hDLL, "Insert");
    PushValue = (PUSHVALUE)GetProcAddress(hDLL, "PushValue");
    Remove = (REMOVE)GetProcAddress(hDLL, "Remove");
    SetTop = (SETTOP)GetProcAddress(hDLL, "SetTop");
    Mark = (MARK)GetProcAddress(hDLL, "Mark");
    Clean = (CLEAN)GetProcAddress(hDLL, "Clean");
    Execute = (EXECUTE)GetProcAddress(hDLL, "Execute");
    if (!Open) {
      FreeLibrary(hDLL);
    }
  }
  
  target->Set(String::NewSymbol("getHandle"), FunctionTemplate::New(GetHandle)->GetFunction());
  target->Set(String::NewSymbol("closeHandle"), FunctionTemplate::New(CloseHandle)->GetFunction());
  target->Set(String::NewSymbol("call"), FunctionTemplate::New(Call)->GetFunction());
}

NODE_MODULE(uodll, Init)