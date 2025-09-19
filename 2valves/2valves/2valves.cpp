#include "NewsScraperFactory.h"
#include <iostream>
#include <vector>
#include <string>

using std::cout;
using std::endl;
using std::cerr;
using std::vector;
using std::string;

/**
 * Fetches and displays news links from a specific scraper
 * @param scraperType The type of scraper to use
 * @param siteName The display name of the news site
 * @return True if links were found and displayed, false otherwise
 */
bool static fetchAndDisplayNews(ScraperType scraperType, const string& siteName) {
    cout << "Fetching news from " << siteName << "..." << endl;

    auto scraper = NewsScraperFactory::createScraper(scraperType);
    if (!scraper) {
        cerr << "Failed to create scraper for " << siteName << endl;
        return false;
    }

    vector<string> links = scraper->fetchAndExtractNewsLinks();
    if (links.empty()) {
        cerr << "No news links found or failed to fetch content from " << siteName << endl;
        return false;
    }

    cout << "Found " << links.size() << " news links:" << endl;
    for (const auto& link : links) {
        cout << link << endl;
    }
    cout << endl; // Add spacing between different sites

	if (links.size() > 0) {
		cout << scraper->fetchAndExtractArticleContent(links[0]) << endl;
	}

    return true;
}

int main() {
    // Define the scrapers to use with their display names
    vector<std::pair<ScraperType, string>> scrapers = {
       /* {ScraperType::INFORMER, "Informer.rs"},
        {ScraperType::KURIR, "Kurir.rs"},*/
        {ScraperType::SCRAPER_021, "021.rs"}//,
        //{ScraperType::N1, "N1info.rs"}
    };

    bool anySuccess = false;

    // Process each scraper
    for (const auto& [scraperType, siteName] : scrapers) {
        if (fetchAndDisplayNews(scraperType, siteName)) {
            anySuccess = true;
        }
    }

    if (!anySuccess) {
        cerr << "Failed to fetch news from any source." << endl;
        return 1;
    }

    cout << "News fetching completed successfully!" << endl;
    return 0;
}