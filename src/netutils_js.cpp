#include <v8.h>
#include <node.h>

#include <set>
#include <iostream>

#include "interface.hpp"

v8::Handle<v8::Value> networkInterfaces(const v8::Arguments& arg)
{
    v8::HandleScope scope;

    class interface::interface* iflist;
    v8::Local<v8::Object> obj = v8::Object::New();

    int i;
    iflist = new interface::interface();
    for (i=0; i<iflist->listsize(); i++) {
        obj->Set(v8::String::NewSymbol(iflist->name(i).c_str()), v8::Array::New());
    }

    v8::Local<v8::Array> v;
    for (i=0; i<iflist->listsize(); i++) {
        v8::Local<v8::Object> tmp = v8::Object::New();
        tmp->Set(v8::String::NewSymbol("address"),
                 v8::String::New(iflist->addr(i).c_str()));
        tmp->Set(v8::String::NewSymbol("family"),
                 v8::String::New(iflist->family(i).c_str()));
        v = v8::Local<v8::Array>::Cast(obj->Get(v8::String::NewSymbol(iflist->name(i).c_str())));
        v->Set(v8::Number::New(v->Length()), tmp); 
    }

    return scope.Close(obj);
}


extern "C" void init(v8::Handle<v8::Object> target)
{
    v8::HandleScope scope;
    target->Set(v8::String::NewSymbol("networkInterfaces"),
                v8::FunctionTemplate::New(networkInterfaces)->GetFunction());
    return;

}

NODE_MODULE(hello, init)
