#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

using std::string;
using std::vector;

/**
 * Abstract interface for news website scrapers
 */
class NewsScraper {
protected:
    const vector<string> PROTEST_KEYWORDS = {
        "protest", "blokad", "student", "šetnja", "demonstrac", "skupštin",
        "vučić", "vucic", "vlada", "vlast", "režim", "rezim", "sns", "opozicij",
        "policij", "žandarm", "zandar", "hapš", "haps", "suzavac",
        "štrajk", "strajk", "bojkot", "autoput", "saobraćaj", "saobracaj",
        "pokret", "aktivis", "građan", "gradan", "korupcij", "cenzur",
        "revolucij", "obojen", "ruš", "rus", "destabiliz", "ustaš",
        "ustas", "teroris", "idiot", "budal", "izdajn", "huligan",
        "ekstremis", "radikal", "fašis", "fasis", "ćac", "cac", "pionirsk",
        "izbor", "vanredn", "list", "saj", "rektor", "dekan", "fasizam",
        "pritvor", "nasil", "plenum", "vjt"
    };

    /**
     * Checks if a URL is relevant for protest-related news
     * @param url The URL to check
     * @return True if the URL contains protest-related keywords
     */
    bool isRelevantForProtests(const std::string& url) const {
        for (const auto& keyword : PROTEST_KEYWORDS) {
            auto it = std::search(url.begin(), url.end(),
                keyword.begin(), keyword.end(),
                [](unsigned char a, unsigned char b) {
                    return std::tolower(a) == std::tolower(b);
                });
            if (it != url.end()) {
                return true;
            }
        }
        return false;
    }

public:
    virtual ~NewsScraper() = default;

    /**
     * Fetches and extracts news article links from the website
     * @param url The URL to scrape (uses default if empty)
     * @return Vector of complete URLs to news articles
     */
    virtual vector<string> fetchAndExtractNewsLinks() = 0;

    /**
     * Extracts news article links from already fetched HTML content
     * @param htmlContent The HTML content as string
     * @return Vector of complete URLs to news articles
     */
    virtual vector<string> extractNewsLinks(const string& htmlContent) = 0;
};