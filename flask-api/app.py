import os
import atexit
from flask import Flask

from config import config
from utils.logging_config import setup_logging
from models.article_processor import ArticleProcessor
from services.scheduler import SchedulerService
from routes.health import register_health_routes
from routes.api import register_api_routes

def create_app(config_name=None):
    """Application factory pattern"""
    
    # Determine configuration
    config_name = config_name or os.environ.get('FLASK_ENV', 'default')
    app_config = config[config_name]
    
    # Setup logging
    logger = setup_logging(app_config.LOG_LEVEL)
    logger.info(f"Starting application with {config_name} configuration")
    
    # Create Flask app
    app = Flask(__name__)
    app.config.from_object(app_config)
    
    # Initialize services
    processor = ArticleProcessor(app_config)
    scheduler_service = SchedulerService(app_config)
    
    # Setup scheduler
    scheduler_service.add_article_processing_job(processor)
    scheduler_service.start()
    
    # Register routes
    register_health_routes(app, scheduler_service)
    register_api_routes(app, processor)
    
    # Cleanup on shutdown
    def cleanup():
        logger.info("Shutting down...")
        scheduler_service.shutdown()
        processor.close_connections()
    
    atexit.register(cleanup)
    
    # Run initial processing
    try:
        processor.process_articles()
    except Exception as e:
        logger.error(f"Initial processing failed: {str(e)}")
    
    return app

if __name__ == '__main__':
    app = create_app()
    
    try:
        app.run(
            debug=app.config['DEBUG'],
            host=app.config['HOST'],
            port=app.config['PORT'],
            use_reloader=False  # Important: prevents duplicate scheduler
        )
    except KeyboardInterrupt:
        pass  # Cleanup handled by atexit