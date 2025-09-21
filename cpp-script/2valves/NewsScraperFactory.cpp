#include "NewsScraperFactory.h"
#include "N1Scraper.h"
#include "Scraper021.h"
#include "InformerScraper.h"
#include "KurirScraper.h"
#include <algorithm>
#include <cctype>

std::unique_ptr<NewsScraper> NewsScraperFactory::createScraper(ScraperType type) {
    switch (type) {
    case ScraperType::N1:
        return std::make_unique<N1Scraper>();
    case ScraperType::SCRAPER_021:
        return std::make_unique<Scraper021>();
    case ScraperType::INFORMER:
        return std::make_unique<InformerScraper>();
    case ScraperType::KURIR:
        return std::make_unique<KurirScraper>();
    default:
        return nullptr;
    }
}

std::unique_ptr<NewsScraper> NewsScraperFactory::createScraper(const std::string& scraperName) {
    ScraperType type;
    if (stringToScraperType(scraperName, type)) {
        return createScraper(type);
    }
    return nullptr;
}

std::string NewsScraperFactory::getScraperTypeName(ScraperType type) {
    switch (type) {
    case ScraperType::N1:
        return "n1";
    case ScraperType::SCRAPER_021:
        return "021";
    case ScraperType::INFORMER:
        return "informer";
    case ScraperType::KURIR:
        return "kurir";
    default:
        return "unknown";
    }
}

bool NewsScraperFactory::stringToScraperType(const std::string& scraperName, ScraperType& outType) {
    // Convert to lowercase for case-insensitive comparison
    std::string lowerName = scraperName;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
        [](unsigned char c) { return std::tolower(c); });

    if (lowerName == "n1") {
        outType = ScraperType::N1;
        return true;
    }
    else if (lowerName == "021") {
        outType = ScraperType::SCRAPER_021;
        return true;
    }
    else if (lowerName == "informer") {
        outType = ScraperType::INFORMER;
        return true;
    }
    else if (lowerName == "kurir") {
        outType = ScraperType::KURIR;
        return true;
    }

    return false;
}