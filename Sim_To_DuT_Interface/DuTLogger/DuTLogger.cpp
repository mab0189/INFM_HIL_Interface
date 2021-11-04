//
// Created by marco on 26.10.21.
//

#include "DuTLogger.h"

// initialize the static handlers
quill::Handler* DuTLogger::consoleHandler = DuTLogger::buildConsoleHandler();
quill::Handler* DuTLogger::consoleFileHandler = DuTLogger::buildFileHandler();

// initialize the static loggers
quill::Logger* DuTLogger::consoleLogger = DuTLogger::createConsoleLogger("consoleLog", false);
quill::Logger* DuTLogger::consoleFileLogger = DuTLogger::createConsoleLogger("consoleFileLog", true);
quill::Logger* DuTLogger::dataLogger = DuTLogger::createDataLogger();

/**
 * Builds a logger for the console. If necessary it the logger will be build with a additional handler for
 * files. This will be configured by the second argument.
 *
 * @param name the name for the logger
 * @param withFileHandler true if the logger also needs a handler for logfiles
 * @return the created logger
 */
quill::Logger* DuTLogger::createConsoleLogger(const char* name, bool withFileHandler) {
    // check if the quill engine is started
    startEngine();

    // check if the new logger needs to write the console log to a file
    // if it needs it, we have to build the logger a different way
    quill::Logger* newLogger;
    if (withFileHandler) {
        newLogger = quill::create_logger(name, {consoleHandler, consoleFileHandler});
    } else {
        newLogger = quill::create_logger(name, consoleHandler);
    }

    // Set the LogLevel (L3 for everything)
    newLogger->set_log_level(quill::LogLevel::TraceL3);

    return newLogger;
}

/**
 * Starts the quill engine. Won't start it twice.
 */
void DuTLogger::startEngine() {
    // start the quill engine if it hasn't been started yet
    if (!startedQuillEngine) {
        quill::enable_console_colours();
        quill::start();

        // remember that we started the engine
        startedQuillEngine = true;
    }
}

quill::Handler* DuTLogger::buildConsoleHandler() {
    // build a handler for the console
    quill::Handler* newHandler = consoleHandler = quill::stdout_handler("consoleHandler");
    newHandler->set_log_level(DEFAULT_CONSOLE_LOG_LEVEL);

    // modify the pattern for the logger
    newHandler->set_pattern(QUILL_STRING("%(ascii_time)  %(level_name): %(message)"),
                                "%D %H:%M:%S.%Qms");
    return newHandler;
}

quill::Handler* DuTLogger::buildFileHandler() {
    // get the path to the log file or create the directory if necessary
    std::string path = DuTLogger::getLoggingPath(LOGGER_TYPE::CONSOLE);
    createDirectoryIfNecessary(path);

    // a second handler for the file is needed
    quill::Handler* newHandler = quill::file_handler(path.append("/Logfile.log"), "w");
    newHandler->set_log_level(DEFAULT_FILE_LOG_LEVEL);

    // modify the pattern for the logger
    newHandler->set_pattern(QUILL_STRING("%(ascii_time)  %(level_name): %(message)"),
                            "%D %H:%M:%S.%Qms");
    // return the new handler
    return newHandler;
}

quill::Logger* DuTLogger::createDataLogger() {
    // get the path to the log file or create the directory if necessary
    std::string path = DuTLogger::getLoggingPath(LOGGER_TYPE::DATA);
    createDirectoryIfNecessary(path);

    // create a file handler to connect quill to a logfile
    quill::Handler* file_handler = quill::file_handler(path.append("/Logfile.log"), "w");

    // configure the pattern of a line
    file_handler->set_pattern(QUILL_STRING("%(ascii_time) %(logger_name) - %(message)"),
            "%D %H:%M:%S.%Qms %z", quill::Timezone::LocalTime);

    // finally, create the logger and return it
    quill::Logger* createdLogger = quill::create_logger("dataLog", file_handler);

    return createdLogger;
}

/**
 * Identifies the path to the log file for the specific typ of logger
 *
 * @param type type of logger
 * @return path to the logfile
 */
std::string DuTLogger::getLoggingPath(LOGGER_TYPE type) {
    // get the path, where this program is running
    std::string path = std::filesystem::current_path();

    // modify the path, so it leads to our logfile
    std::string result = path.substr(0, path.find_last_of("/"));

    // get the right path for the logger
    if (type == LOGGER_TYPE::DATA) {
        result.append(PATH_DATA_LOG);
    } else {
        result.append(PATH_CONSOLE_LOG);
    }
    // return the path
    return result;
}

void DuTLogger::createDirectoryIfNecessary(const std::string path) {
    // check if the directory is created in the file system
    std::filesystem::create_directories(path);
}

/**
 * Change the logging level for a logger. Because there are multiple loggers you have to
 * select for which type of logger you want to change the level.
 * @param type the type of logger
 * @param level new level for logging
 */
void DuTLogger::changeLogLevel(LOG_LEVEL_CHANGE_ON type, LOG_LEVEL level) {
    quill::Handler* handler = DuTLogger::getHandlerType(type);

    switch (level) {
        case LOG_LEVEL::NONE:
            handler->set_log_level(quill::LogLevel::None);
            break;

        case LOG_LEVEL::DEBUG:
            handler->set_log_level(quill::LogLevel::Debug);
            break;

        case LOG_LEVEL::INFO:
            handler->set_log_level(quill::LogLevel::Info);
            break;

        case LOG_LEVEL::WARNING:
            handler->set_log_level(quill::LogLevel::Warning);
            break;

        case LOG_LEVEL::ERROR:
            handler->set_log_level(quill::LogLevel::Error);
            break;

        case LOG_LEVEL::CRITICAL:
            handler->set_log_level(quill::LogLevel::Critical);
            break;

        default:
            logMessage("Can not change LogLevel! Invalid argument!", LOG_LEVEL::ERROR, true);
            handler->set_log_level(quill::LogLevel::TraceL3);
            break;
    }
}

/**
 * Returns the right quill handler for the given type of logger
 * @param type type of logger
 * @return connected handler
 */
quill::Handler* DuTLogger::getHandlerType(LOG_LEVEL_CHANGE_ON type) {
    quill::Handler* handler;

    switch (type) {
        case LOG_LEVEL_CHANGE_ON::CONSOLE_LOG:
            handler = consoleHandler;
            break;

        case LOG_LEVEL_CHANGE_ON::FILE_LOG:
            handler = consoleFileHandler;
            break;

        default:
            throw std::invalid_argument("Internal Error! Unknown Typ of <LOG_LEVEL_CHANGED_ON> appeared.");
    }

    return handler;
}

/**
 * Logs the message with consideration of the log level. Please notice that this function will also log the message
 * into a logfile if the file logger accepts the log level. If you explicitly don't want to log this message into the
 * logfile, please use another function.
 * @param msg message that should be logged
 * @param level the logging level for this message
 */
void DuTLogger::logMessage(std::string msg, LOG_LEVEL level) {
    // hand the message with the right level to the quill framework
    // the user didn't choose if it has to be logged in the logfile
    // -> try to log it with the level to the file, the level on the fileLogger will decide if the msg will be written
    logWithLevel(consoleFileLogger, msg, level);
}

/**
 * Logs the message with consideration of the log level. This function gives you additionally the possibility
 * to choose with the 3rd parameter if you want to log the message into the logfile or explicitly not.
 * Please notice that there will be no file logging if the logger won't support the log level, even if you
 * parse 'true' on the 3rd argument.
 * @param msg message that should be logged
 * @param level the logging level for this message
 * @param writeToFile false, if you don't want to log into file
 */
void DuTLogger::logMessage(std::string msg, LOG_LEVEL level, bool writeToFile) {
    // check if we have to write the message to the LogFile
    if (writeToFile) {
        logWithLevel(consoleFileLogger, msg, level);
    } else {
        logWithLevel(consoleLogger, msg, level);
    }
}

void DuTLogger::logWithLevel(quill::Logger* log, std::string msg, LOG_LEVEL level) {
    // parse the message to the right will logger with the given level
    switch (level) {
        case LOG_LEVEL::NONE:
            LOG_WARNING(log, "Can't log this message, because the chosen Log_Level is <NONE>");
            break;

        case LOG_LEVEL::DEBUG:
            LOG_DEBUG(log, "{}", msg);
            break;

        case LOG_LEVEL::INFO:
            LOG_INFO(log, "{}", msg);
            break;

        case LOG_LEVEL::WARNING:
            LOG_WARNING(log, "{}", msg);
            break;

        case LOG_LEVEL::ERROR:
            LOG_ERROR(log, "{}", msg);
            break;

        case LOG_LEVEL::CRITICAL:
            LOG_CRITICAL(log, "{}", msg);
            break;

        default:
            throw std::invalid_argument("Parsed unknown LOG_LEVEL <" + msg + ">");
    }
}