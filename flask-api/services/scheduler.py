import logging
from apscheduler.schedulers.background import BackgroundScheduler

logger = logging.getLogger(__name__)

class SchedulerService:
    """Service for managing background jobs"""
    
    def __init__(self, config):
        self.config = config
        self.scheduler = BackgroundScheduler()
        self.job_id = 'article_scraper_job'
    
    def add_article_processing_job(self, processor):
        """Add article processing job to scheduler"""
        self.scheduler.add_job(
            func=processor.process_articles,
            trigger="interval",
            hours=self.config.SCRAPER_INTERVAL_HOURS,
            id=self.job_id
        )
        logger.info(f"Article processing job scheduled every {self.config.SCRAPER_INTERVAL_HOURS} hours")
    
    def start(self):
        """Start the scheduler"""
        self.scheduler.start()
        logger.info("Scheduler started")
    
    def shutdown(self):
        """Shutdown the scheduler"""
        self.scheduler.shutdown()
        logger.info("Scheduler stopped")
    
    def get_next_run_time(self):
        """Get next scheduled run time"""
        job = self.scheduler.get_job(self.job_id)
        return job.next_run_time.isoformat() if job and job.next_run_time else None