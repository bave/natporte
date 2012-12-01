#ifndef LOG_HPP
#define LOG_HPP

#include <syslog.h>
#include <iostream>

//-- openlog option
// LOG_PID    個々のメッセージに PID を含める
// LOG_CONS   エラーがあれば,システムロガーに送る一方でシステムコンソールにも直接書く
// LOG_NDELAY ログ記録用プログラムとの接続を即座に開始する(通常は,最初のメッセージが記録される時に接続を開く)
// LOG_NOWAIT メッセージを記録する際に生成される子プロセスの終了を待たない(Linuxでは無効)
// LOG_ODELAY LOG_NDELAYの反対. syslog()が呼ばれるまで,接続の開始を行わない(デフォルト)
// LOG_PERROR 標準エラー出力にも出力する(POSIX.1-2001 では定義されていない)

//-- level
// 0 LOG_EMRG    システム使用不可メッセージ
// 1 LOG_ALERT   直ちに対処が必要なメッセージ
// 2 LOG_CRIT    危険通知メッセージ
// 3 LOG_ERR     エラー状態通知メッセージ
// 4 LOG_WARNING ワーニング状態通知メッセージ
// 5 LON_NOTICE  通常かつ重要な状態の通知メッセージ
// 6 LOG_INFO    インフォメーションメッセージ
// 7 LOG_DEBUG   デバッグレベルのメッセージ

//-- facility
// auth, LOG_AUTH
// authpriv, LOG_AUTHPRIV
// cron", LOG_CRON
// daemon", LOG_DAEMON
// ftp", LOG_FTP
// kern", LOG_KERN
// lpr", LOG_LPR
// mail", LOG_MAIL
// mark", INTERNAL_MARK
// news", LOG_NEWS
// security", LOG_AUTH
// syslog", LOG_SYSLOG
// user", LOG_USER
// uucp", LOG_UUCP
// local0", LOG_LOCAL0
// local1", LOG_LOCAL1
// local2", LOG_LOCAL2
// local3", LOG_LOCAL3
// local4", LOG_LOCAL4
// local5", LOG_LOCAL5
// local6", LOG_LOCAL6
// local7", LOG_LOCAL7

using namespace std;

const char *pname = NULL;
extern bool verbose;
extern class log* l;

#define DM(MSG)                                 \
if (verbose) {                                  \
    char str[BUFSIZ];                           \
    snprintf(str, sizeof(str), "%s:%s[%d]: %s", \
            __FILE__, __func__, __LINE__, MSG); \
    l->output(str);                             \
}

#define PERROR(MSG)                                    \
do {                                                   \
    char str[BUFSIZ];                                  \
    snprintf(str, sizeof(str), "%s %s:%s[%d]: %s",     \
            pname ,__FILE__, __func__, __LINE__, MSG); \
    perror(str);                                       \
} while(0);

class log
{
    public:
        // prototype function
        log(char* name, int f, int l);
        ~log();
        void output(string& message);
        void output(const char* message);
        void setLevel(int l);
        int getLevel(void);

        // member declaration
        string prog_name;
        int level;
        int facility;
};

log::log(char* name, int f, int l)
{
    prog_name = name;
    facility = f;
    level = l;
    openlog(name, LOG_PID | LOG_PERROR, facility);
};

void log::output(string& message)
{
    syslog(level, message.c_str(), NULL);
    return;
};

void log::output(const char* message)
{
    syslog(level, message, NULL);
    return;
};

void log::setLevel(int l)
{
    level = l;
    return;
}

int log::getLevel(void)
{
    return level;
}

log::~log()
{
    closelog();
    return;
};

#endif // LOG_HPP
