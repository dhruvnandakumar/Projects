# Preprocess timestamp and release_date to be consistent
date_to_sec <- function(date_str) {
  as.integer(as.POSIXct(as.Date(date_str, format='%d-%b-%Y')))
}

preprocess <- function(ratings) {
  ####### 1. Right Types ####################################################
  # str(ratings) # shows user_id, movie_id are integer but should be factor
  ratings$user_id <- as.factor(ratings$user_id)
  ratings$movie_id <- as.factor(ratings$movie_id)
  
  # release_date is a factor but should be in the same format as timestamp
  ratings$release_date <- apply(ratings[c('release_date')], 1, date_to_sec)
  
  # zipcode as factor with 795 levels too much
  # only use first two digits of zipcode
  ratings$zip_code <- substr(as.character(ratings$zip_code),1,2)
  ratings$zip_code <- as.factor(ratings$zip_code)
  
  ####### 2. Missing Values ##################################################
  # remove overvations with missing values for release_date
  missing_dates <- ratings$release_date %>% is.na() %>% which()
  ratings <- ratings[-missing_dates,]
  
  ####### 3. Noninformative Features ########################################
  # Remove noninfomative predictors
  drops <- c('movie_title','video_release_date','imdb_url')
  ratings <- ratings[ , !names(ratings) %in% drops]
  return(ratings)
}

