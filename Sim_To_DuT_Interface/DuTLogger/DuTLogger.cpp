//
// Created by marco on 26.10.21.
//

#include "DuTLogger.h"

// initialize the logging paths
std::string DuTLogger::currentLogpathConsole = initializeLoggingPath(LOGGER_TYPE::CONSOLE);
std::string DuTLogger::currentLogpathData = initializeLoggingPath(LOGGER_TYPE::DATA);

// initialize the static handlers
quill::Handler* DuTLogger::consoleHandler = DuTLogger::buildConsoleHandler();
quill::Handler* DuTLogger::consoleFileHandler = DuTLogger::buildFileHandler();

// initialize the static loggers
quill::Logger* DuTLogger::consoleLogger = DuTLogger::createConsoleLogger("consoleLog", false);
quill::Logger* DuTLogger::consoleFileLogger = DuTLogger::createConsoleLogger("consoleFileLog", true);
quill::Logger* DuTLogger::dataLogger = DuTLogger::createDataLogger();


/**
 * Starts the quill engine. Won't start the engine again if it's already running.
 */
void DuTLogger::startEngine() {
    // initialize everything if it hasn't been done yet
    if (!startedQuillEngine) {
        // start the quill engine
        quill::start();

        // remember that we started the engine and checked everything
        startedQuillEngine = true;
    }
}

/**
 * Creates a handler to write messages in the console.
 * This handler can be connected to a logger.
 *
 * @return a handler to log messages in the console
 */
quill::Handler* DuTLogger::buildConsoleHandler() {
    // build a handler for the console
    quill::Handler* newHandler = quill::stdout_handler("consoleHandler");
    newHandler->set_log_level(DEFAULT_CONSOLE_LOG_LEVEL);

    // modify the pattern for the logger
    newHandler->set_pattern(QUILL_STRING("%(ascii_time)  %(level_name): %(message)"),
                                "%D %H:%M:%S.%Qms");
    return newHandler;
}

/**
 * Creates a handler to write messages into a file.
 * This handler can be connected to a logger.
 * Depending on the configuration the handler will create a new file for every start or try to append on the current one.
 *
 * @return a handler to log messages in a file
 */
quill::Handler* DuTLogger::buildFileHandler() {
    // a second handler for the file is needed
    std::string basicPath = currentLogpathConsole + "/Logfile_" + getCurrentTimestamp() + ".log";
    quill::Handler* newHandler = quill::file_handler(basicPath, FILE_MODE_CONSOLE,
                                                     quill::FilenameAppend::None);
    newHandler->set_log_level(DEFAULT_FILE_LOG_LEVEL);

    // modify the pattern for the logger
    newHandler->set_pattern(QUILL_STRING("%(ascii_time)  %(level_name): %(message)"),
                            "%D %H:%M:%S.%Qms");
    // return the new handler
    return newHandler;
}

/**
 * Builds a logger for the console. If necessary the logger will be build with an additional handler for
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
    // if needed, we have to build the logger in a different way
    quill::Logger* newLogger;
    if (withFileHandler) {
        newLogger = quill::create_logger(name, {consoleHandler, consoleFileHandler});

        // check if we might have to delete old logging folders
        removeOldLogfiles(currentLogpathConsole);
    } else {
        newLogger = quill::create_logger(name, consoleHandler);
    }

    // Set the LogLevel (L3 for everything)
    newLogger->set_log_level(quill::LogLevel::TraceL3);

    return newLogger;
}

/**
 * Creates a logger to log a specific data object (like events; not normal messages!) into a file.
 * The logger will be build with all necessary handlers.
 * Depending on the configuration the handler will create a new file for every start or try to append on the current one.
 *
 * @return a logger to log data objects into a file
 */
quill::Logger* DuTLogger::createDataLogger() {
    // create a file handler to connect quill to a logfile
    std::string basicPath = currentLogpathData + "/Logfile_" + getCurrentTimestamp() + ".log";
    quill::Handler* file_handler = quill::file_handler(basicPath, FILE_MODE_DATA,quill::FilenameAppend::None);

    // configure the pattern of a line
    file_handler->set_pattern(QUILL_STRING("%(message)"));

    // finally, create the logger and return it
    quill::Logger* createdLogger = quill::create_logger("dataLog", file_handler);

    // check if we might have to delete old logging folders
    removeOldLogfiles(currentLogpathData);

    return createdLogger;
}

/**
 * This function returns a valid path to the logfiles. To handle this task it will read the configurated path.
 * If necessary it will create the path.
 * @param type the type of logger
 * @return a valid path for creating and modifying logfiles
 */
std::string DuTLogger::initializeLoggingPath(LOGGER_TYPE type) {
    // get the wished logging path for this type of logger
    std::string path = getLoggingPath(type);

    // create the log directory if it doesn't exist
    std::filesystem::create_directories(path);
    return path;
}

/**
 * Identifies the path to the log file for the specific typ of logger by using the underlying path configuration.
 * The function can handle absolute and relative paths from the configuration.
 *
 * @param type  - type of logger
 * @return path to the logfile
 */
std::string DuTLogger::getLoggingPath(LOGGER_TYPE type) {
    // Check if the user provided an absolute path for logging
    // If this is true -> remove the first identifier
    if (type == LOGGER_TYPE::CONSOLE && PATH_CONSOLE_LOG.at(0) == '#') {
        return PATH_CONSOLE_LOG.substr(1, PATH_CONSOLE_LOG.size()-1);
    }
    if (type == LOGGER_TYPE::DATA && PATH_DATA_LOG.at(0) == '#') {
        return PATH_DATA_LOG.substr(1, PATH_DATA_LOG.size()-1);
    }

    // the user provided a relative path -> build the relative path
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

/**
 * This function should be called on every startup of the application.
 * It deletes old logfiles in the given logging directory.
 *
 * Which files will be deleted depends on the configured backup count. If the current number of files is higher
 * than the allowed number by the backup count, old files will be deleted until we have the allowed number again.
 *
 * Please notice that on every start of the application the file handlers will automatically create a new file,
 * if there isn't already a file for the day.
 *
 * @param directory old logfiles will be deleted in this directory
 */
void DuTLogger::removeOldLogfiles(std::string directory) {
    // collect all files under this directory in a list
    std::list<std::string> allLogFiles;
    for (const auto & entry : std::filesystem::directory_iterator(directory)) {
        allLogFiles.push_back(entry.path());
    }

    // check if we have more files than allowed by the backup count
    if (allLogFiles.size() > FILE_BACKUP_COUNT) {
        // sort the list alphabetically so the oldest file will be the first one
        allLogFiles.sort();

        // get the iterator for the list and search for old files to remove them
        // remove so much files until we have the accepted number again
        // remove from the front, because there will be the old files
        std::list<std::string>::iterator iter = allLogFiles.begin();
        for (int i = 0; i < (allLogFiles.size() - FILE_BACKUP_COUNT); i++) {
            // get the path to the file and remove it
            std::string file = *iter;
            std::remove(file.c_str());          // cast string to const char*

            // set the iterator on the next object for the next iteration
            std::advance(iter, 1);
        }
    }
}

/**
 * Change the logging level for a specific logger. Because there are multiple loggers you have to
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
 * logfile, please use the function below.
 *
 * @see void DuTLogger::logMessage(std::string msg, LOG_LEVEL level, bool doNotWriteIntoFile)
 * @param msg message that should be logged
 * @param level the logging level for this message
 */
void DuTLogger::logMessage(std::string msg, LOG_LEVEL level) {
    // hand the message with the right level to the quill framework.
    // if the user didn't choose to log the message into the logfile -> try to log it with the level
    // to the file, the level on the fileLogger will decide if the msg will be written
    logWithLevel(consoleFileLogger, msg, level);
}

/**
 * Logs the message with consideration of the log level. This function gives you additionally the possibility
 * to prevent that the logger will write your message into the logfile. So if you set the 3rd parameter to 'false'
 * the logger won't log your message into the file even if the log level would have accepted it.
 *
 * @param msg message that should be logged
 * @param level the logging level for this message
 * @param doNotWriteIntoFile true, if you don't want to log into the logfile
 */
void DuTLogger::logMessage(std::string msg, LOG_LEVEL level, bool doNotWriteIntoFile) {
    // check if we have to write the message to the LogFile
    if (doNotWriteIntoFile) {
        logWithLevel(consoleLogger, msg, level);
    } else {
        logWithLevel(consoleFileLogger, msg, level);
    }
}

/**
 * This help function will parse the message to the quill engine.
 *
 * @param log the specific logger that should be used
 * @param msg the message that should be logged
 * @param level the logging level
 */
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
            throw std::invalid_argument("Parsed unknown LOG_LEVEL!");
    }
}

/**
 * This function returns a string with the current time. This string can be used to name files.
 *
 * @return the current time as string
 */
std::string DuTLogger::getCurrentTimestamp() {
    // look up the local time
    auto t = std::time(nullptr);
    auto timer = *std::localtime(&t);

    // write it formatted in a stream and convert that to a string so we can return it
    std::ostringstream oss;
    oss << std::put_time(&timer, "%Y-%m-%d_%H-%M-%S");
    return oss.str();
}

/**
 * This function logs the event to the data logfiles. There is no need to define a logging level for this operation.
 *
 * @param event This event will be logged.
 */
void DuTLogger::logEvent(sim_interface::SimEvent event) {
    // check if we already printed the header to the file.
    // Because of troubles with the quill engine we can't write the header in the file when we're building logger
    if (!csvHeaderPrinted) {
        // log the header
        LOG_INFO(dataLogger, "{},{},{},{}", "Operation", "Value", "Origin", "Timestamp");
        // remember that we have logged the header
        csvHeaderPrinted = true;
    }

    // because the value of the event can have different types, it is necessary to find out what type it is.
    // so test through all known cases in order. If you find the right one -> log it
    if (event.value.type() == typeid(int)) {
        int value = boost::get<int>(event.value);
        LOG_INFO(dataLogger, "{},{},{},{}", event.operation, value, event.origin, event.current);
    } else if (event.value.type() == typeid(double)) {
        double value = boost::get<double>(event.value);
        LOG_INFO(dataLogger, "{},{},{},{}", event.operation, value, event.origin, event.current);
    } else if (event.value.type() == typeid(std::string)) {
        std::string value = boost::get<std::string>(event.value);
        LOG_INFO(dataLogger, "{},{},{},{}", event.operation, value, event.origin, event.current);
    } else {
        logMessage("Can't log event: Unknown type for the value of the event.", LOG_LEVEL::ERROR);
    }
}