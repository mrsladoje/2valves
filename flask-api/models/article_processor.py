import logging
from services.database import DatabaseService
from services.cpp_builder import CppBuilderService

logger = logging.getLogger(__name__)

class ArticleProcessor:
    """Main class for processing articles"""
    
    def __init__(self, config):
        self.config = config
        self.database = DatabaseService(config)
        self.cpp_builder = CppBuilderService(config)
    
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
    
    def get_stats(self):
        """Get database statistics"""
        return self.database.get_stats()
    
    def close_connections(self):
        """Close all connections"""
        self.database.close_connection()