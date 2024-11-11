#include "Logger.h"

#ifndef NDEBUG

#pragma comment(lib, "shell32.lib")

#define BUFFER_SIZE 1024

#define COLOR_INFO 7
#define COLOR_WARN 14
#define COLOR_ERR 12
#define COLOR_TIME 3

Logger::Logger() {
    this->loglvl = ERR;
    this->includeTime = PARTIAL;
    this->logFileSpawned = false;
    this->failedToOpenFile = false;
    this->DbgIsInit = false;
    this->spawnedCMD = false;
    this->hConsole = nullptr;
    this->logfile = nullptr;

    //this->spawnCMD();
}

void Logger::spawnCMD() {

    if (this->spawnedCMD) {
        logwarn("Commandline already exists. Ignoring.");
        return;
    }

    AllocConsole();

    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);

    freopen_s(&fp, "CONIN$", "r", stdin);

    std::ios::sync_with_stdio();

    SetConsoleTitle(L"CS2 trainer logs");
    this->hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    this->spawnedCMD = true;
}

void Logger::println(Loglvl lvl, std::string filename, int line, std::string func, std::string format, ...) {
    if (lvl < this->loglvl) {
       return;
    }

    if (!spawnedCMD) {
        Logger::getLogger().spawnCMD();
    }

    if (!logFileSpawned && !failedToOpenFile) {
        Logger::getLogger().spawnLogFile();
    }

    int r;
    va_list args;
    va_start(args, format);
    char tmp[BUFFER_SIZE];
    r = vsnprintf(tmp, BUFFER_SIZE, format.c_str(), args);
    va_end(args);
    std::string str(tmp);
    
    switch (lvl)
    {
    case INFO:
        str = std::string("INFO : ") + str;
        break;
    case WARN:
        str = std::string("WARNING : ") + str;
        break;
    case ERR:
        str = std::string("ERROR : ") + str;
        break;
    default:
        break;
    }


    auto now = std::chrono::system_clock::now();

    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_time_t);

    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream oss;

    switch (this->includeTime)
    {
    case PARTIAL:
        oss << "[" << std::put_time(&now_tm, "%H:%M") << "] ";
        break;
    case PRECISE:
        oss << "[" << std::put_time(&now_tm, "%H:%M:%S") << "] ";
        break;
    case FULL:
        oss << "[" << std::put_time(&now_tm, "%H:%M:%S") << '.'
            << std::setw(3) << std::setfill('0') << now_ms.count() << "] ";
        break;
    case NONE:
    default:
        /* DO NOTHING */
        break;
    }

    std::string additional = "[" + filename + ":" + std::to_string(line) + ":" + func + "()] ";



    SetConsoleTextAttribute(hConsole, COLOR_TIME);
    std::cout << oss.str();
    std::cout << additional;

    switch (lvl)
    {
    case INFO:
        SetConsoleTextAttribute(hConsole, COLOR_INFO);
        break;
    case WARN:
        SetConsoleTextAttribute(hConsole, COLOR_WARN);
        break;
    case ERR:
        SetConsoleTextAttribute(hConsole, COLOR_ERR);
        break;
    default:
        break;
    }

    std::cout<< str << std::endl;


    if (logFileSpawned) {
        //this->logFile << oss.str() << str << std::endl;
        fprintf(logfile, "%s%s%s\n", oss.str().c_str(), additional.c_str(), str.c_str());
    }

}

Logger& Logger::getLogger() {
    static Logger instance;
    return instance;
}

void Logger::setLoglvl(Loglvl lvl) {
    this->loglvl = lvl;
}

void Logger::setTimePrecision(TimePrecision precision) {
    this->includeTime = precision;
}

void Logger::spawnLogFile() {

    if (failedToOpenFile) {
        return;
    }

    if (logFileSpawned) {
        return;
    }

    char *tmp;
    size_t len;
    _dupenv_s(&tmp, &len, "USERPROFILE");
    std::string path = tmp;

    free(tmp);

    if (len==0) {
        return;
    }
    std::string dir = path + std::string("\\cs2logs");

    if(!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
        std::string command = "";
        command += "mkdir ";
        command += dir;
        system(command.c_str());
        if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
            failedToOpenFile = true;
            return;
        }

    }
    path = path + std::string("\\cs2logs\\") + std::to_string(time(NULL)) + std::string(".log");
    

    int err = fopen_s(&logfile, path.c_str(), "w");
    if (err==0) {
        this->logFileSpawned = true;
        loginfo("log file path `%s`", path.c_str());
    }
    else {
        this->failedToOpenFile = true;
        logerr("Failed to create log file at `%s` with errno `%d`. TRY manualy creating directory `%s`. Probable cause is insufficient permissions.", path.c_str(),err, dir.c_str());
    }
}

Logger::~Logger() {
    if (this->logFileSpawned) {
        fclose(logfile);
        this->logFileSpawned = false;
    }

    // Close the FILE pointers
    fclose(stdout);
    fclose(stderr);
    fclose(stdin);

    // Free the console
    FreeConsole();
}

#endif // !NDEBUG