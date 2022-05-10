# Logger
- Чтобы открыть лог файл ```void Logger::OpenLogFile(const char* filename)``` (по умолчанию **log/log.txt**, без вызова этой функции логи пишутся в **stdout**)
- После использования нужно вызвать ```void Logger::Close()``` для сброса буффера и закрытия файла.
- Доступны уровни: LOG_INFO, LOG_DEBUG, LOG_ERROR, LOG_WARN
- Аргументы - имя модуля, затем как у fmt::print()

### Sample
```C++
#include "core/logger.hpp"

int main() {
  Logger::OpenLogFile("log/log.txt");

  LOG_INFO(main, "Hi from main {}", 11);

  Logger::Close();
}
```