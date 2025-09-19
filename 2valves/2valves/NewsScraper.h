#pragma once
#include <string>
#include <vector>
#include <lexbor/html/html.h>
#include <lexbor/dom/dom.h>

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
        "revolucij", "obojen", "ruš", "destabiliz", "ustaš",
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
    bool isRelevantForProtests(const std::string& url) const;

    /**
     * Checks if a node has the specified class name
     * @param element The element to check
     * @param className The class name to look for
     * @return True if element has the class
     */
    bool hasClass(lxb_dom_element_t* element, const string& className) const;

    /**
     * Gets the href attribute value from an anchor element
     * @param element The anchor element
     * @return The href value, or empty string if not found
     */
    string getHref(lxb_dom_element_t* element) const;

    /**
     * Gets the tag name of an element
     * @param element The element
     * @return The tag name as string
     */
    string getTagName(lxb_dom_element_t* element) const;

    /**
     * Check if element has a specific attribute with a specific value
     * @param element The DOM element to check
     * @param attributeName The attribute name to check for
     * @param attributeValue The expected attribute value
     * @return True if element has the attribute with the specified value
     */
    bool hasAttribute(lxb_dom_element_t* element, const string& attributeName, const string& attributeValue) const;

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

