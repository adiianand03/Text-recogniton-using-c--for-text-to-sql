#include <iostream>
#include <string>
#include <regex>
#include <iomanip>
#include <sstream>
#include <ctime>

using namespace std;

ostringstream getFormattedDate(int year, int month, int day) {
    ostringstream oss;
    oss << setw(4) << setfill('0') << year << "-"
        << setw(2) << setfill('0') << month << "-"
        << setw(2) << setfill('0') << day;
    return oss;
}

string convertDate(const string& input) {
    time_t now = time(0);
    tm* ltm = localtime(&now);

    int month, day, year;
    string monthStr, dayStr, yearStr, result = input;
    ostringstream oss, oss_first, oss_sec;
    regex pattern;
    regex datePattern1(R"(([Jj]an(?:uary)?|[Ff]eb(?:uary)?|[Mm]ar(?:ch)?|[Aa]pr(?:ril)?|[Mm]ay|[Jj]un(?:e)?|[Jj]ul(?:y)?|[Aa]ug(?:est)?|[Ss]ep(?:tember)?|[Oo]ct(?:ober)?|[Nn]ov(?:ember)?|[Dd]ec(?:ember)?) (\d{1,2}(?:th|st|nd|rd)?)(?: (\d{4}))?)");
    regex datePattern2(R"((\d{1,2}(?:th|st|nd|rd)?) ([Jj]an(?:uary)?|[Ff]eb(?:uary)?|[Mm]ar(?:ch)?|[Aa]pr(?:ril)?|[Mm]ay|[Jj]un(?:e)?|[Jj]ul(?:y)?|[Aa]ug(?:est)?|[Ss]ep(?:tember)?|[Oo]ct(?:ober)?|[Nn]ov(?:ember)?|[Dd]ec(?:ember)?)(?: (\d{4}))?)");
    regex todayPattern(R"([Tt]oday)");
    regex yesterdayPattern(R"([Yy]esterday)");
    regex lastyearPattern(R"(last year)");
    regex thisyearPattern(R"(this year)");
    regex dateRangePattern(R"((.*?)(?:\s+and\s+|\s+to\s+)(.*))");
    regex datePattern3(R"((\d{1,2}) ([Jj]an(?:uary)?|[Ff]eb(?:uary)?|[Mm]ar(?:ch)?|[Aa]pr(?:ril)?|[Mm]ay|[Jj]un(?:e)?|[Jj]ul(?:y)?|[Aa]ug(?:est)?|[Ss]ep(?:tember)?|[Oo]ct(?:ober)?|[Nn]ov(?:ember)?|[Dd]ec(?:ember)?) (\d{4}))");

    smatch match;
    if (regex_search(result, match, todayPattern)) {
        oss = getFormattedDate(1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);
        result = regex_replace(result, todayPattern, "\"" + oss.str() + "\"");
        return result;
    }
    if (regex_search(result, match, yesterdayPattern)) {
        now = now - (24 * 60 * 60);
        ltm = localtime(&now);
        oss = getFormattedDate(1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);
        result = regex_replace(result, yesterdayPattern, "\"" + oss.str() + "\"");
        return result;
    }
    if (regex_search(result, match, lastyearPattern)) {
        oss_first = getFormattedDate(1900 + ltm->tm_year - 1, 1, 1);
        oss_sec = getFormattedDate(1900 + ltm->tm_year - 1, 12, 31);
        result = regex_replace(result, lastyearPattern, "between \"" + oss_first.str() + "\"" + " and \"" + oss_sec.str() + "\"");
        return result;
    }
    if (regex_search(result, match, thisyearPattern)) {
        oss_first = getFormattedDate(1900 + ltm->tm_year, 1, 1);
        oss_sec = getFormattedDate(1900 + ltm->tm_year, 12, 31);
        result = regex_replace(result, thisyearPattern, "between \"" + oss_first.str() + "\"" + " and \"" + oss_sec.str() + "\"");
        return result;
    }

    if (regex_search(result, match, dateRangePattern)) {
        string startDateStr = match[1].str();
        string endDateStr = match[2].str();
        string formattedStartDate = convertDate(startDateStr);
        string formattedEndDate = convertDate(endDateStr);
        result = regex_replace(result, dateRangePattern, "between \"" + formattedStartDate + "\"" + " and \"" + formattedEndDate + "\"");
        return result;
    }

    if (regex_search(result, match, datePattern3)) {
        dayStr = match[1].str();
        monthStr = match[2].str();
        yearStr = match[3].str();
        pattern = datePattern3;
    } else if (regex_search(result, match, datePattern1)) {
        monthStr = match[1].str();
        dayStr = match[2].str();
        yearStr = match[3].str();
        pattern = datePattern1;
    } else if (regex_search(result, match, datePattern2)) {
        monthStr = match[2].str();
        dayStr = match[1].str();
        yearStr = match[3].str();
        pattern = datePattern2;
    } else {
        return result;
    }

    if (regex_search(monthStr, regex("[Jj]an(uary)?"))) month = 1;
    else if (regex_search(monthStr, regex("[Ff]eb(?:uary)?"))) month = 2;
    else if (regex_search(monthStr, regex("[Mm]ar(?:ch)?"))) month = 3;
    else if (regex_search(monthStr, regex("[Aa]pr(?:ril)?"))) month = 4;
    else if (regex_search(monthStr, regex("[Mm]ay"))) month = 5;
    else if (regex_search(monthStr, regex("[Jj]un(?:e)?"))) month = 6;
    else if (regex_search(monthStr, regex("[Jj]ul(?:y)?"))) month = 7;
    else if (regex_search(monthStr, regex("[Aa]ug(?:est)?"))) month = 8;
    else if (regex_search(monthStr, regex("[Ss]ep(?:tember)?"))) month = 9;
    else if (regex_search(monthStr, regex("[Oo]ct(?:ober)?"))) month = 10;
    else if (regex_search(monthStr, regex("[Nn]ov(?:ember)?"))) month = 11;
    else if (regex_search(monthStr, regex("[Dd]ec(?:ember)?"))) month = 12;
    else {
        return result;
    }

    day = stoi(dayStr);
    year = (yearStr.empty() ? 1900 + ltm->tm_year : stoi(yearStr));

    oss = getFormattedDate(year, month, day);
    result = regex_replace(result, pattern, "\"" + oss.str() + "\"");
    return result;
}

// int main() {
//     cout << convertDate("Jan 26th") << endl;
//     cout << convertDate("Jan 26") << endl;
//     cout << convertDate("January 26") << endl;
//     cout << convertDate("jan 26") << endl;
//     cout << convertDate("nov 26") << endl;
//     cout << convertDate("26th Jan") << endl;
//     cout << convertDate("26 jan and 14 jun") << endl;
//     cout << convertDate("10th oct and 15th oct") << endl;
//     cout << convertDate("today") << endl;
//     cout << convertDate("yesterday") << endl;
//     cout << convertDate("last year") << endl;
//     cout << convertDate("this year") << endl;
//     cout << convertDate("jan 26 to jun 4") << endl;
//     cout << convertDate("jan 26 to 4 jun ") << endl;
//     cout << convertDate("Jan 26 2002") << endl;
//     cout << convertDate("jan 26 2002") << endl;
//     cout << convertDate("26 jan 2002") << endl;
//     cout << convertDate("26 jan 2022 to 26 jan 2023") << endl;
//     cout << convertDate("26 jan 2002 to 10 oct") << endl;
//     return 0;
// }
