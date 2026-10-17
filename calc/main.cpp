#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

struct Timestamp
{
  int year, month, day, hour, minute, second;
};

struct Event
{
  long long epoch; // segundos desde 1970-01-01 (hora local tratada como UTC)
  int line;        // linha do CSV, pra avisar com precisão
};

// Só aceita exatamente AAAA-MM-DDTHH:MM:SS-03:00
bool looksValid(const std::string &s)
{
  if (s.size() != 25)
    return false;
  const std::string mask = "dddd-dd-ddTdd:dd:dd";
  for (size_t i = 0; i < mask.size(); i++)
  {
    if (mask[i] == 'd' ? !std::isdigit(static_cast<unsigned char>(s[i])) : s[i] != mask[i])
      return false;
  }
  return s.compare(19, 6, "-03:00") == 0;
}

Timestamp parseTimestamp(const std::string &line)
{
  Timestamp t;
  t.year = std::stoi(line.substr(0, 4));
  t.month = std::stoi(line.substr(5, 2));
  t.day = std::stoi(line.substr(8, 2));
  t.hour = std::stoi(line.substr(11, 2));
  t.minute = std::stoi(line.substr(14, 2));
  t.second = std::stoi(line.substr(17, 2));
  return t;
}

int secondsOfDay(const Timestamp &t)
{
  return t.hour * 3600 + t.minute * 60 + t.second;
}

// Dias entre 1970-01-01 e a data (Howard Hinnant, days_from_civil)
long long daysFromCivil(int y, int m, int d)
{
  y -= m <= 2;
  const long long era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = static_cast<unsigned>(y - era * 400);
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + static_cast<long long>(doe) - 719468;
}

// O inverso: número do dia -> ano, mês, dia (Hinnant, civil_from_days)
void civilFromDays(long long z, int &y, int &m, int &d)
{
  z += 719468;
  const long long era = (z >= 0 ? z : z - 146096) / 146097;
  const unsigned doe = static_cast<unsigned>(z - era * 146097);
  const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  const unsigned mp = (5 * doy + 2) / 153;
  d = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
  m = static_cast<int>(mp < 10 ? mp + 3 : mp - 9);
  y = static_cast<int>(yoe) + static_cast<int>(era * 400) + (m <= 2 ? 1 : 0);
}

long long toEpochSeconds(const Timestamp &t)
{
  return daysFromCivil(t.year, t.month, t.day) * 86400LL + secondsOfDay(t);
}

std::string dayLabel(long long day)
{
  int y, m, d;
  civilFromDays(day, y, m, d);
  char buf[16];
  std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", y, m, d);
  return buf;
}

std::string formatHMS(long long s)
{
  char buf[32];
  std::snprintf(buf, sizeof(buf), "%lld:%02lld:%02lld", s / 3600, (s % 3600) / 60, s % 60);
  return buf;
}

int main(int argc, char **argv)
{
  const std::string path = argc > 1 ? argv[1] : "../samples/dia-normal.csv";
  std::ifstream inputFile(path);

  if (!inputFile.is_open())
  {
    std::cout << "Erro ao abrir o arquivo: " << path << "\n";
    return 1;
  }

  int warnings = 0;
  std::vector<Event> events;
  std::string line;
  int lineNumber = 0;

  while (std::getline(inputFile, line))
  {
    lineNumber++;
    if (!line.empty() && line.back() == '\r')
      line.pop_back(); // CRLF do Windows

    if (!looksValid(line))
    {
      std::cout << "AVISO: linha " << lineNumber << " mal formatada, ignorada: \"" << line << "\"\n";
      warnings++;
      continue;
    }
    events.push_back({toEpochSeconds(parseTimestamp(line)), lineNumber});
  }

  if (events.empty())
  {
    std::cout << "Nenhum evento valido no arquivo.\n";
    return warnings > 0 ? 2 : 0;
  }

  std::map<long long, long long> perDay; // número do dia -> segundos

  for (size_t i = 0; i + 1 < events.size(); i += 2)
  {
    const Event &in = events[i];
    const Event &out = events[i + 1];

    if (out.epoch < in.epoch)
    {
      std::cout << "AVISO: eventos fora de ordem nas linhas " << in.line << " e " << out.line << ", par ignorado\n";
      warnings++;
      continue;
    }
    if (out.epoch == in.epoch)
    {
      std::cout << "AVISO: eventos no mesmo segundo nas linhas " << in.line << " e " << out.line << " (aperto fantasma?), par ignorado\n";
      warnings++;
      continue;
    }

    // Parte o intervalo na meia-noite: cada dia recebe só o que foi trabalhado nele
    long long cur = in.epoch;
    while (cur < out.epoch)
    {
      const long long day = cur / 86400;
      const long long chunkEnd = std::min(out.epoch, (day + 1) * 86400);
      perDay[day] += chunkEnd - cur;
      cur = chunkEnd;
    }
  }

  if (events.size() % 2 != 0)
  {
    const Event &last = events.back();
    std::cout << "AVISO: numero impar de eventos. A linha " << last.line
              << " (" << dayLabel(last.epoch / 86400) << ") ficou sem saida.\n";
    warnings++;
  }

  std::map<std::string, long long> perMonth;

  std::cout << "\nPor dia:\n";
  for (const auto &entry : perDay)
  {
    const std::string label = dayLabel(entry.first);
    std::cout << label << "  " << formatHMS(entry.second) << "\n";
    perMonth[label.substr(0, 7)] += entry.second;
  }

  std::cout << "\nPor mes:\n";
  for (const auto &entry : perMonth)
  {
    std::cout << entry.first << "  " << formatHMS(entry.second) << "\n";
  }

  if (warnings > 0)
  {
    std::cout << "\nATENCAO: " << warnings << " aviso(s). Confira antes de confiar nos totais.\n";
    return 2;
  }
  return 0;
}