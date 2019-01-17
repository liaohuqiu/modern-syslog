#include <stdio.h>
#include <node.h>
#include <nan.h>
#include <syslog.h>

using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Local;
using v8::Null;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Value;

namespace {

class Worker: public Nan::AsyncWorker {
    public:
        Worker(Nan::Callback *callback, char* ident, int option, int facility, int priority, char* message)
            : Nan::AsyncWorker(callback), ident(ident), option(option), facility(facility), priority(priority), message(message) {
            }
        ~Worker() {
            delete[] ident;
            delete[] message;
        }

        void Execute() {
            openlog(ident, option, facility);
            syslog(priority, "%s", message);
            closelog();
        }

        void HandleOKCallback() {
            Nan::HandleScope scope;

            if (callback) {
                callback->Call(0, NULL);
            }
        };

    private:
        char* ident;
        int option;
        int facility;
        int priority;
        char* message;
};

static char* dupBuf(const Handle<Value>& arg) {
    const char* mem = node::Buffer::Data(arg);
    size_t memsz = node::Buffer::Length(arg);
    char* s = new char[memsz + 1];
    memcpy(s, mem, memsz);
    s[memsz] = 0;
    return s;
}

static char* dupStr(const Local<String>& m) {
    if(m.IsEmpty())
        return NULL;

    // Exact calculation of UTF length involves double traversal. Avoid this
    // because we know UTF8 expansion is < 4 bytes out per byte in.
    char* s = new char[m->Length() * 4];
    m->WriteUtf8(s);
    return s;
}

// wrap: void syslog(const char *ident, int option, int facility, int priority, const char *format, ...);
NAN_METHOD(SysLog) {

    char* ident = NULL;
    if (node::Buffer::HasInstance(info[0])) {
        ident = dupBuf(info[0]);
    } else {
        ident = dupStr(info[0]->ToString());
    }

    int option = info[1]->Int32Value();
    int facility = info[2]->Int32Value();
    int priority = info[3]->Int32Value();

    char* message = NULL;
    if (node::Buffer::HasInstance(info[4])) {
        message = dupBuf(info[4]);
    } else {
        message = dupStr(info[4]->ToString());
    }

    Nan::Callback *callback = NULL;
    if (info[5]->IsFunction())
        callback = new Nan::Callback(info[5].As<Function>());

    if (message) {
        Nan::AsyncQueueWorker(new Worker(callback, ident, option, facility, priority, message));
    } else if (callback) {
        callback->Call(0, NULL);
        delete callback;
    }

    return;
}

NAN_MODULE_INIT(Init) {
    Nan::Export(target, "syslog", SysLog);

    Local<Object> where = Nan::New<Object>();
#define DEFINE(N) Nan::Set(where, Nan::New<String>(#N).ToLocalChecked(), Nan::New<Number>(N))

    // option argument is an OR of any of these:
    Nan::Set(target, Nan::New<String>("option").ToLocalChecked(), where = Nan::New<Object>());
    DEFINE(LOG_CONS);
    DEFINE(LOG_NDELAY);
    DEFINE(LOG_ODELAY);
#ifndef LOG_PERROR
// not defined on Solaris but we want the exported object to be consistent
#define LOG_PERROR 0x0 // no-op
#endif
    DEFINE(LOG_PERROR);
    DEFINE(LOG_PID);
    DEFINE(LOG_NOWAIT);

    // facility argument is any ONE of these:
    Nan::Set(target, Nan::New<String>("facility").ToLocalChecked(), where = Nan::New<Object>());
    DEFINE(LOG_AUTH);
#ifdef LOG_AUTHPRIV
    DEFINE(LOG_AUTHPRIV);
#endif
    DEFINE(LOG_CRON);
    DEFINE(LOG_DAEMON);
#ifdef LOG_FTP
    DEFINE(LOG_FTP);
#endif
    DEFINE(LOG_KERN);
    DEFINE(LOG_LOCAL0);
    DEFINE(LOG_LOCAL1);
    DEFINE(LOG_LOCAL2);
    DEFINE(LOG_LOCAL3);
    DEFINE(LOG_LOCAL4);
    DEFINE(LOG_LOCAL5);
    DEFINE(LOG_LOCAL6);
    DEFINE(LOG_LOCAL7);
    DEFINE(LOG_LPR);
    DEFINE(LOG_MAIL);
    DEFINE(LOG_NEWS);
    DEFINE(LOG_SYSLOG);
    DEFINE(LOG_USER);
    DEFINE(LOG_UUCP);

    // priority argument to syslog() is an OR of a facility and ONE log level:
    Nan::Set(target, Nan::New<String>("level").ToLocalChecked(), where = Nan::New<Object>());
    DEFINE(LOG_EMERG);
    DEFINE(LOG_ALERT);
    DEFINE(LOG_CRIT);
    DEFINE(LOG_ERR);
    DEFINE(LOG_WARNING);
    DEFINE(LOG_NOTICE);
    DEFINE(LOG_INFO);
    DEFINE(LOG_DEBUG);
}
}

NODE_MODULE(core, Init);
