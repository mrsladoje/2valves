#pragma once
#include "NewsScraper.h"
#include <memory>
#include <string>

/**
 * Enum class for different news scraper types
 */
enum class ScraperType {
    N1,
    SCRAPER_021,
    INFORMER,
    KURIR
};

/**
 * Factory class for creating news scraper instances
 */
class NewsScraperFactory {
public:
    /**
     * Creates a news scraper instance based on the specified type
     * @param type The type of scraper to create
     * @return Unique pointer to the created scraper, or nullptr if type is invalid
     */
    static std::unique_ptr<NewsScraper> createScraper(ScraperType type);

    /**
     * Creates a news scraper instance based on a string identifier
     * @param scraperName The name of the scraper ("n1", "021", "informer", "kurir")
     * @return Unique pointer to the created scraper, or nullptr if name is invalid
     */
    static std::unique_ptr<NewsScraper> createScraper(const std::string& scraperName);

    /**
     * Gets the string representation of a scraper type
     * @param type The scraper type
     * @return String name of the scraper type
     */
    static std::string getScraperTypeName(ScraperType type);

    /**
     * Converts a string name to ScraperType enum
     * @param scraperName The string name of the scraper
     * @param outType Reference to store the converted type
     * @return True if conversion was successful, false otherwise
     */
    static bool stringToScraperType(const std::string& scraperName, ScraperType& outType);

private:
    // Private constructor to prevent instantiation
    NewsScraperFactory() = default;
};