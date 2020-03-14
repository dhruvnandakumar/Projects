# Import data and create ratings matrix

# Revreive data from specified csv file 
ratings_from_csv <- function(file_path) {
  ratings <- read.csv(file_path, header=FALSE, sep='\t')
  names(ratings) <- c('user_id', 'movie_id', 'rating', 'timestamp')
  return(ratings)
}

get_data <- function() {
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
  
  # Create data frame for user data
  user_info <- read.csv('ml-100k/u.user', header=FALSE, sep='|')
  names(user_info) <- c('user_id', 'age', 'gender', 'occupation', 'zip_code')
  
  # Merge movie user data into rating data
  ratings <- merge(ratings, movie_info, by.x='movie_id', by.y='movie_id')
  ratings <- merge(ratings, user_info, by.x='user_id', by.y='user_id')
  return(ratings)
}
