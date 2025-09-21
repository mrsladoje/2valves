import logging
import json
from datetime import datetime
from pymongo import MongoClient
from pymongo.errors import PyMongoError

logger = logging.getLogger(__name__)

class DatabaseService:
    """Service for handling MongoDB operations"""
    
    def __init__(self, config):
        self.config = config
        self.client = None
        self.db = None
        self.collection = None
        self._connect()
    
    def _connect(self):
        """Establish MongoDB connection"""
        try:
            self.client = MongoClient(self.config.MONGO_URI)
            self.db = self.client[self.config.DATABASE_NAME]
            self.collection = self.db[self.config.COLLECTION_NAME]
            
            # Test connection
            self.client.admin.command('ping')
            logger.info(f"Connected to MongoDB: {self.config.DATABASE_NAME}")
            
        except PyMongoError as e:
            logger.error(f"Failed to connect to MongoDB: {str(e)}")
            raise
    
    def save_articles(self, articles_json):
        """Parse JSON and save articles to MongoDB"""
        try:
            articles = json.loads(articles_json)
            
            # Handle both single article and array of articles
            if isinstance(articles, dict):
                articles = [articles]
            elif not isinstance(articles, list):
                logger.error("Invalid JSON format - expected object or array")
                return False
            
            saved_count = 0
            for article in articles:
                # Add timestamp when saved
                article['saved_at'] = datetime.utcnow()
                
                # Use upsert to avoid duplicates based on _id (URL)
                result = self.collection.update_one(
                    {"_id": article.get("_id")},
                    {"$set": article},
                    upsert=True
                )
                
                if result.upserted_id or result.modified_count > 0:
                    saved_count += 1
            
            logger.info(f"Successfully saved/updated {saved_count} articles")
            return True
            
        except json.JSONDecodeError as e:
            logger.error(f"Error parsing JSON: {str(e)}")
            return False
        except PyMongoError as e:
            logger.error(f"Database error: {str(e)}")
            return False
        except Exception as e:
            logger.error(f"Error saving to database: {str(e)}")
            return False
    
    def get_all_articles(self, limit=100):
        """Get all articles from database"""
        try:
            articles = list(
                self.collection.find(
                    {}, 
                    {"_id": 1, "date": 1, "saved_at": 1}
                ).limit(limit)
            )
            return articles
        except PyMongoError as e:
            logger.error(f"Error retrieving articles: {str(e)}")
            return []
    
    def get_article_by_id(self, article_id):
        """Get specific article by ID"""
        try:
            return self.collection.find_one({"_id": article_id})
        except PyMongoError as e:
            logger.error(f"Error retrieving article {article_id}: {str(e)}")
            return None
    
    def get_articles_by_date(self, date_str):
        """Get all articles for a specific date (YYYY-MM-DD format)"""
        try:
            from datetime import datetime
            
            # Parse the date string
            try:
                target_date = datetime.strptime(date_str, "%Y-%m-%d")
            except ValueError:
                logger.error(f"Invalid date format: {date_str}. Expected YYYY-MM-DD")
                return None
            
            # Create date range for the entire day
            start_date = target_date.replace(hour=0, minute=0, second=0, microsecond=0)
            end_date = target_date.replace(hour=23, minute=59, second=59, microsecond=999999)
            
            # Query articles saved on that date
            articles = list(
                self.collection.find({
                    "saved_at": {
                        "$gte": start_date,
                        "$lte": end_date
                    }
                }).sort("saved_at", -1)  # Most recent first
            )
            
            logger.info(f"Found {len(articles)} articles for date {date_str}")
            return articles
            
        except PyMongoError as e:
            logger.error(f"Error retrieving articles for date {date_str}: {str(e)}")
            return None
        except Exception as e:
            logger.error(f"Unexpected error retrieving articles for date {date_str}: {str(e)}")
            return None
    
    def get_stats(self):
        """Get database statistics"""
        try:
            total_articles = self.collection.count_documents({})
            latest_article = self.collection.find_one(
                {}, 
                sort=[("saved_at", -1)]
            )
            
            return {
                "total_articles": total_articles,
                "latest_article_saved": latest_article.get("saved_at").isoformat() if latest_article else None,
                "database": self.config.DATABASE_NAME,
                "collection": self.config.COLLECTION_NAME
            }
        except PyMongoError as e:
            logger.error(f"Error getting stats: {str(e)}")
            return {
                "total_articles": 0,
                "latest_article_saved": None,
                "database": self.config.DATABASE_NAME,
                "collection": self.config.COLLECTION_NAME,
                "error": str(e)
            }
    
    def close_connection(self):
        """Close MongoDB connection"""
        if self.client:
            self.client.close()
            logger.info("MongoDB connection closed")