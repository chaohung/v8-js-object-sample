#include <cstdio>

#include "libplatform/libplatform.h"
#include "v8.h"

class Hoge {
public:
    Hoge() { printf("Hoge constructor.\n"); }
    ~Hoge() { printf("Hoge destructor.\n"); }

    static void New(v8::FunctionCallbackInfo<v8::Value> const& info) {
        Hoge* h = new Hoge();
        info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), h));
        info.GetReturnValue().Set(info.This());

        // destructor callback
        v8::Persistent<v8::Object> holder(info.GetIsolate(), info.This());
        holder.SetWeak(h, WeakCallback, v8::WeakCallbackType::kParameter);
    }

    static void WeakCallback(v8::WeakCallbackInfo<Hoge> const& info) {
        Hoge *h = info.GetParameter();
        delete h;
    }

    static void GetNum(v8::Local<v8::String> property, v8::PropertyCallbackInfo<v8::Value> const& info) {
        v8::Local<v8::Object> self = info.Holder();
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
        void* p = wrap->Value();
        int value = static_cast<Hoge*>(p)->num;
        info.GetReturnValue().Set(value);
        printf("Hoge::GetNum.\n");
    }

    static void SetNum(v8::Local<v8::String> property, v8::Local<v8::Value> value,
        v8::PropertyCallbackInfo<void> const& info) {
        v8::Local<v8::Object> self = info.Holder();
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
        void* p = wrap->Value();
        static_cast<Hoge*>(p)->num = value->Int32Value();
        printf("Hoge::SetNum.\n");
    }

public:
    int num;
};

std::string read_file(char const* path) {
    FILE* fp = fopen(path, "rb");
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);
    std::string buf(size, 0);
    fread((void*)buf.data(), 1, size, fp);
    fclose(fp);
    return buf;
}

void Test(v8::FunctionCallbackInfo<v8::Value> const& info) {
    printf("test\n");
}

void Print(v8::FunctionCallbackInfo<v8::Value> const& info) {
    bool first = true;
    for (int i = 0; i < info.Length(); i++) {
        v8::HandleScope handle_scope(info.GetIsolate());
        if (first) {
            first = false;
        } else {
            printf(" ");
        }
        v8::String::Utf8Value str(info.GetIsolate(), info[i]);
        printf("%s", *str);
    }
    printf("\n");
    fflush(stdout);
}

v8::Local<v8::Context> create_context(v8::Isolate* isolate) {
    v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);

    global->Set(v8::String::NewFromUtf8(isolate, "test", v8::NewStringType::kNormal).ToLocalChecked(),
        v8::FunctionTemplate::New(isolate, Test));

    global->Set(v8::String::NewFromUtf8(isolate, "print", v8::NewStringType::kNormal).ToLocalChecked(),
        v8::FunctionTemplate::New(isolate, Print));

    v8::Local<v8::FunctionTemplate> hoge_template = v8::FunctionTemplate::New(isolate, Hoge::New);
    hoge_template->InstanceTemplate()->SetInternalFieldCount(1);
    hoge_template->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "num"),
        Hoge::GetNum, Hoge::SetNum);
    global->Set(v8::String::NewFromUtf8(isolate, "Hoge", v8::NewStringType::kNormal).ToLocalChecked(),
        hoge_template);

    return v8::Context::New(isolate, NULL, global);
}

int main(int argc, char* argv[]) {
    std::string input = read_file("input.js");

    char const* builtin_path = getenv("BUILTIN_PATH");
    v8::V8::InitializeICUDefaultLocation(builtin_path);
    v8::V8::InitializeExternalStartupData(builtin_path);
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator =
        v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(create_params);
    {
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = create_context(isolate);
        v8::Context::Scope context_scope(context);
        v8::Local<v8::String> source =
            v8::String::NewFromUtf8(isolate, input.c_str(),
                    v8::NewStringType::kNormal)
            .ToLocalChecked();

        v8::Local<v8::Script> script =
            v8::Script::Compile(context, source).ToLocalChecked();
        v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

        //v8::String::Utf8Value utf8(isolate, result);
        //printf("%s\n", *utf8);
    }

    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete create_params.array_buffer_allocator;
    return 0;
}
