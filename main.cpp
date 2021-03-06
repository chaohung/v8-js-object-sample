#include <cstdio>
#include <string>
#include <vector>

#include "libplatform/libplatform.h"
#include "v8.h"

class Hoge {
public:
    Hoge(int num, char const* str, uint8_t* buf_data, size_t buf_size, bool buf_owner) :
        num_(num), str_(str), buf_data_(buf_data), buf_size_(buf_size), buf_owner_(buf_owner) {
            printf("Hoge constructor.\n");
    }
    ~Hoge() {
        printf("Hoge destructor.\n");
        if (buf_owner_) delete[] buf_data_;
    }

    void dump() {
        printf("\033[31m");
        printf("hoge address: %p\n", this);
        printf("num: %d\n", num_);
        printf("str: %s\n", str_.c_str());
        printf("buf: ");
        for (int i = 0; i < buf_size_; i++) {
            printf("%d,", buf_data_[i]);
        } printf("\n");
        printf("\033[00m");
    }

    static void New(v8::FunctionCallbackInfo<v8::Value> const& info) {
        int num = info[0]->Int32Value();
        v8::String::Utf8Value utf8_str(info.GetIsolate(), info[1]);
        char const* str = *utf8_str;
        v8::Local<v8::Uint8Array> uint8_array_object = v8::Local<v8::Uint8Array>::Cast(info[2]);
        v8::ArrayBuffer::Contents content;
        bool buf_owner;
        if (uint8_array_object->Buffer()->IsExternal()) {
            content = uint8_array_object->Buffer()->GetContents();
            buf_owner = false;
        } else {
            content = uint8_array_object->Buffer()->Externalize();
            buf_owner = true;
        }
        Hoge* h = new Hoge(num, str, static_cast<uint8_t*>(content.Data()), content.ByteLength(), buf_owner);
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
        int value = static_cast<Hoge*>(p)->num_;
        info.GetReturnValue().Set(value);
        printf("Hoge::GetNum.\n");
    }

    static void SetNum(v8::Local<v8::String> property, v8::Local<v8::Value> value,
        v8::PropertyCallbackInfo<void> const& info) {
        v8::Local<v8::Object> self = info.Holder();
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
        void* p = wrap->Value();
        static_cast<Hoge*>(p)->num_ = value->Int32Value();
        printf("Hoge::SetNum.\n");
    }

    static void GetStr(v8::Local<v8::String> property, v8::PropertyCallbackInfo<v8::Value> const& info) {
        v8::Local<v8::Object> self = info.Holder();
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
        void* p = wrap->Value();
        auto& str = static_cast<Hoge*>(p)->str_;
        v8::Local<v8::String> value = v8::String::NewFromUtf8(info.GetIsolate(), str.c_str(),
            v8::NewStringType::kNormal, str.length()).ToLocalChecked();
        info.GetReturnValue().Set(value);
        printf("Hoge::GetStr.\n");
    }

    static void SetStr(v8::Local<v8::String> property, v8::Local<v8::Value> value,
        v8::PropertyCallbackInfo<void> const& info) {
        v8::Local<v8::Object> self = info.Holder();
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
        void* p = wrap->Value();
        static_cast<Hoge*>(p)->str_ = *v8::String::Utf8Value(info.GetIsolate(), value);
        printf("Hoge::SetStr.\n");
    }

    static void GetBuf(v8::Local<v8::String> property, v8::PropertyCallbackInfo<v8::Value> const& info) {
        v8::Local<v8::Object> self = info.Holder();
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
        void* p = wrap->Value();
        Hoge* h = static_cast<Hoge*>(p);
        v8::Local<v8::ArrayBuffer> array_buf = v8::ArrayBuffer::New(info.GetIsolate(),
            h->buf_data_, h->buf_size_);
        v8::Local<v8::Uint8Array> value = v8::Uint8Array::New(array_buf, 0, h->buf_size_);
        info.GetReturnValue().Set(value);
        printf("Hoge::GetBuf.\n");
    }

    static void Dump(v8::FunctionCallbackInfo<v8::Value> const& info) {
        v8::Local<v8::Object> self = info.Holder();
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
        void* p = wrap->Value();
        static_cast<Hoge*>(p)->dump();
    }

public:
    int num_;
    std::string str_;
    uint8_t* buf_data_;
    size_t buf_size_;
    bool buf_owner_;
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

void GlobalAccess(v8::FunctionCallbackInfo<v8::Value> const& info) {
    v8::String::Utf8Value str(info.This()->Get(
        v8::String::NewFromUtf8(
            info.GetIsolate(), "node", v8::NewStringType::kNormal).ToLocalChecked()));
    printf("node: %s\n", *str);
}

void Callback(v8::FunctionCallbackInfo<v8::Value> const& info) {
	v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(info[0]);
    v8::Local<v8::Value> num = v8::Integer::New(info.GetIsolate(), 98765);
    func->Call(info.This(), 1, &num);
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

    global->Set(
        v8::String::NewFromUtf8(isolate, "test", v8::NewStringType::kNormal).ToLocalChecked(),
        v8::FunctionTemplate::New(isolate, Test));

    global->Set(
        v8::String::NewFromUtf8(isolate, "print", v8::NewStringType::kNormal).ToLocalChecked(),
        v8::FunctionTemplate::New(isolate, Print));

    global->Set(
        v8::String::NewFromUtf8(
            isolate, "global_access", v8::NewStringType::kNormal).ToLocalChecked(),
        v8::FunctionTemplate::New(isolate, GlobalAccess));

    global->Set(
        v8::String::NewFromUtf8(isolate, "native_object", v8::NewStringType::kNormal).ToLocalChecked(),
        v8::Integer::New(isolate, 12345));

	global->Set(
		v8::String::NewFromUtf8(isolate, "callback", v8::NewStringType::kNormal).ToLocalChecked(),
        v8::FunctionTemplate::New(isolate, Callback));

    v8::Local<v8::FunctionTemplate> hoge_template = v8::FunctionTemplate::New(isolate, Hoge::New);
    hoge_template->InstanceTemplate()->SetInternalFieldCount(1);
    hoge_template->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "num"),
        Hoge::GetNum, Hoge::SetNum);
    hoge_template->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "str"),
        Hoge::GetStr, Hoge::SetStr);
    hoge_template->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "buf"),
        Hoge::GetBuf);
    hoge_template->InstanceTemplate()->Set(
        v8::String::NewFromUtf8(isolate, "dump", v8::NewStringType::kNormal).ToLocalChecked(),
        v8::FunctionTemplate::New(isolate, Hoge::Dump));
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

        v8::Local<v8::Integer> num = v8::Integer::New(isolate, 123456789);
        printf("[native] num test: %d\n", num->Uint32Value());
        v8::Local<v8::String> str = v8::String::NewFromUtf8(isolate, "hogehoge");
        v8::String::Utf8Value utf8_str(str);
        printf("[native] str test: %s\n", *utf8_str);

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
