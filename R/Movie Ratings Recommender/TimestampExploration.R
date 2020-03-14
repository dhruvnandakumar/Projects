# TimestampExploration.R

library(ggplot2)
library(scales)

ratings_from_csv <- function(file_path) {
    ratings <- read.csv(file_path, header=FALSE, sep='\t')
    names(ratings) <- c('user_id', 'movie_id', 'rating', 'timestamp')
    return(ratings)
}

# Create data frames for ratings
ratings_train <- ratings_from_csv('ml-100k/ua.base')
ratings_test <- ratings_from_csv('ml-100k/ua.test')
ratings <- rbind(ratings_train, ratings_test)

# Create data frame for movie data
movie_info <- read.csv('ml-100k/u.item', header=FALSE, sep='|')
names(movie_info) <- c('movie_id', 'movie_title', 'release_date',
                       'video_release_date', 'imdb_url', 'unknown', 'action',
                       'adventure', 'animation', 'childrens', 'comedy', 'crime',
                       'documentary', 'drama', 'fantasy', 'filmnoir', 'horror',
                       'musical', 'mystery', 'romance', 'scifi', 'thriller',
                       'war', 'western')

mcols <- c('movie_id', 'movie_title', 'release_date')
ratings <- merge(ratings, movie_info[mcols], by.x='movie_id', by.y='movie_id')

# Convert to timestamps
date_to_posixct <- function(date_str) {
    as.POSIXct(as.Date(date_str, format='%d-%b-%Y', tz='UTC'))
}

ratings$release_date <- apply(ratings[c('release_date')], 1, date_to_posixct)
ratings$timestamp <- apply(ratings[c('timestamp')], 1,
	                       as.POSIXct, origin='1970-01-01')

# Remove incomplete rows
ratings <- ratings[complete.cases(ratings),]

# New dataframe with column for time before release date
early_ratings <- ratings[ratings$release_date > ratings$timestamp,]
early_ratings$time_delta = early_ratings$release_date - early_ratings$timestamp

# Generate plots
ggplot(data=early_ratings, aes(x=as.POSIXct(timestamp, origin='1970-01-01'), fill=movie_title)) + geom_histogram(show.legend=FALSE) +
scale_x_datetime(labels=date_format('%d-%b-%Y')) +
ggtitle('Ratings Before Movie Release') + xlab('rating timestamp') +
theme(plot.title = element_text(hjust = 0.5))

ggplot(data=early_ratings, aes(x=time_delta/86400.0, fill=movie_title)) +
geom_histogram(show.legend=FALSE) +
ggtitle('Ratings Days Before Movie Release') + xlab('days before movie release date') +
theme(plot.title = element_text(hjust = 0.5))

# Apt Pupil was by far the most rated movie before it's release
# Checking number of ratings before release vs. after
ap_before = early_ratings[early_ratings$movie_id == 315,]
ap_after = ratings[ratings$movie_id == 315 && ratings$timestamp >= ratings$release_date,]
nrow(ap_before)
nrow(ap_after)
# 160 -> 0 (All ratings in dataset were before the release date)
# Reading further, the release date of Apt Pupil was after the region in which the ratings were taken from
# The README says during the seven-month period from September 19th, 1997 through April 22nd, 1998
# Apt Pupil was released October 23, 1998
# That would explain why there aren't any ratings after Apt Pupil's release date in the dataset
# It doesn't explain why there are so many early ratings for Apt Pupil
