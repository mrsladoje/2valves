import logging
from services.database import DatabaseService
from services.cpp_builder import CppBuilderService
from services.gemini_service import GeminiService

logger = logging.getLogger(__name__)

class ArticleProcessor:
    """Main class for processing articles"""
    
    def __init__(self, config):
        self.config = config
        self.database = DatabaseService(config)
        self.cpp_builder = CppBuilderService(config)
        
        # Initialize Gemini service if API key is available
        try:
            self.gemini = GeminiService(config)
        except ValueError as e:
            logger.warning(f"Gemini service not available: {str(e)}")
            self.gemini = None
    
    def process_articles(self):
        """Main function to run C++ script and save articles"""
        logger.info("Starting article processing job...")
        
        # Run C++ script
        json_output = self.cpp_builder.run_executable()
        if not json_output:
            logger.error("Failed to get output from C++ script")
            return False
        
        # Save to database
        success = self.database.save_articles(json_output)
        if success:
            logger.info("Article processing completed successfully")
        else:
            logger.error("Failed to save articles to database")
        
        return success
    
    def build_cpp_project(self):
        """Build C++ project manually"""
        return self.cpp_builder.build_project()
    
    def get_all_articles(self, limit=100):
        """Get all articles from database"""
        return self.database.get_all_articles(limit)
    
    def get_article_by_id(self, article_id):
        """Get specific article by ID"""
        return self.database.get_article_by_id(article_id)
    
    def get_articles_by_date(self, date_str):
        """Get articles for a specific date"""
        return self.database.get_articles_by_date(date_str)
    
    def analyze_news_coverage(self, date_str):
        """Analyze news coverage for a specific date using Gemini AI"""
        if not self.gemini:
            return {"error": "Gemini service not available. Please set GEMINI_API_KEY environment variable."}
        
        # Get articles for the specified date
        articles = self.database.get_articles_by_date(date_str)
        if articles is None:
            return {"error": "Invalid date format. Use YYYY-MM-DD"}
        
        # Perform analysis using Gemini
        return self.gemini.analyze_news_coverage(date_str, articles)
    
    def get_stats(self):
        """Get database statistics"""
        return self.database.get_stats()
    
    def close_connections(self):
        """Close all connections"""
        self.database.close_connection()