#include <iostream>
#include <string>
#include <unistd.h>
#include <signal.h>

#include "interfaces.h"
#include "version.h"
#include "misc.h"
#include "socket.h"
#include "webget.h"
#include "logger.h"

extern std::string pref_path, access_token, listen_address, gen_profile, managed_config_prefix;
extern bool api_mode, generator_mode, cfw_child_process, update_ruleset_on_request;
extern int listen_port, max_concurrent_threads, max_pending_connections;
extern string_array rulesets;
extern std::vector<ruleset_content> ruleset_content_array;

#ifndef _WIN32
void SetConsoleTitle(const std::string &title)
{
    system(std::string("echo \"\\033]0;" + title + "\\007\\c\"").data());
}
#endif // _WIN32

void setcd(std::string &file)
{
    char szTemp[1024] = {}, filename[256] = {};
    std::string path;
#ifdef _WIN32
    char *pname = NULL;
    DWORD retVal = GetFullPathName(file.data(), 1023, szTemp, &pname);
    if(!retVal)
        return;
    strcpy(filename, pname);
    strrchr(szTemp, '\\')[1] = '\0';
#else
    char *ret = realpath(file.data(), szTemp);
    if(ret == NULL)
        return;
    ret = strcpy(filename, strrchr(szTemp, '/') + 1);
    if(ret == NULL)
        return;
    strrchr(szTemp, '/')[1] = '\0';
#endif // _WIN32
    file.assign(filename);
    path.assign(szTemp);
    chdir(path.data());
}

void chkArg(int argc, char *argv[])
{
    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-cfw") == 0)
        {
            cfw_child_process = true;
            update_ruleset_on_request = true;
        }
        else if(strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0)
        {
            if(i < argc - 1)
                pref_path.assign(argv[++i]);
        }
        else if(strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--gen") == 0)
        {
            generator_mode = true;
        }
        else if(strcmp(argv[i], "--artifact") == 0)
        {
            if(i < argc - 1)
                gen_profile.assign(argv[++i]);
        }
        else if(strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--log") == 0)
        {
            if(i < argc - 1)
                if(freopen(argv[++i], "a", stderr) == NULL)
                    std::cerr<<"Error redirecting output to file.\n";
        }
    }
}


int main(int argc, char *argv[])
{
#ifndef _DEBUG
    std::string prgpath = argv[0];
    setcd(prgpath); //first switch to program directory
#endif // _DEBUG
    if(fileExist("pref.yml"))
        pref_path = "pref.yml";
    chkArg(argc, argv);
    setcd(pref_path); //then switch to pref directory
    writeLog(0, "SubConverter " VERSION " starting up..", LOG_LEVEL_INFO);
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
    {
        //std::cerr<<"WSAStartup failed.\n";
        writeLog(0, "WSAStartup failed.", LOG_LEVEL_FATAL);
        return 1;
    }
    UINT origcp = GetConsoleOutputCP();
    defer(SetConsoleOutputCP(origcp);)
    SetConsoleOutputCP(65001);
#else
    signal(SIGPIPE, SIG_IGN);
    signal(SIGABRT, SIG_IGN);
#endif // _WIN32

    SetConsoleTitle("SubConverter " VERSION);
    readConf();
    if(!update_ruleset_on_request)
        refreshRulesets(rulesets, ruleset_content_array);
    generateBase();

    return simpleGenerator();

}
