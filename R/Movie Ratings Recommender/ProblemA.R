library(ggplot2)  
library(tidyverse)  # install.packages('tidyverse') for easy data manipulation
library(lubridate)  # install.packages('lubridate') for processing Date
library(corrplot)   # intall.packages('corrplot') for correlation plot
source('data.R')
source('preprocess.R')
source('add_features.R')
source('remove_noninformative_features.R')
source('remove_correlated_features.R')
source('categorical_exploration.R')

#### ADD HELPER FUNCTIONS HERE ####

# create ratings data matrix
ratings <- get_data()

# remove missing values, uninformative features, and correct data types
ratings <- preprocess(ratings)

# add features that could be more informative than original features
# ADDED: "release_year", "release_month", "timestamp_year", 
#        "timestamp_month", "time_difference"
ratings <- add_features(ratings)

# move rating column to last column for better visualization in corr plots
no_rating_col <- ratings[, !names(ratings) %in% "rating"]
ratings <- cbind(no_rating_col, rating = ratings$rating)

# display correlation plot of all numeric features
numeric_features <- select_if(ratings, is.numeric) %>% names() 
corr_matrix <- cor(ratings[,names(ratings) %in% numeric_features])
corrplot(corr_matrix, method="color")

# remove features that have very low correlation with rating
# REMOVED: 'timestamp', 'unknown', 'adventure','animation', 
#          'musical','thriller','western'
ratings <- remove_noninformative_features(ratings)

# display correlation plot with noninformative features removed
numeric_features2 <- select_if(ratings, is.numeric) %>% names() 
corr_matrix2 <- cor(ratings[,names(ratings) %in% numeric_features2])
corrplot(corr_matrix2, method="color")

# remove less informative feature of highly correlated feature pairs
# REMOVED: "comedy","action","release_date","fantasy","time_difference"
ratings <- remove_correlated_features(ratings)

# display final correlation plot with features removed
numeric_features3 <- select_if(ratings, is.numeric) %>% names() 
corr_matrix3 <- cor(ratings[,names(ratings) %in% numeric_features3])
corrplot(corr_matrix3, method="color")

# DISPLAY ALL 3 CORRELATION PLOTS
par(mfrow=c(1,3))
corrplot(corr_matrix, method="color")
corrplot(corr_matrix2, method="color")
corrplot(corr_matrix3, method="color")

# EXPLORE NUMERIC: Release_year, drama, war appears have the highest corr 

# EXPLORE CATEGORICAL: user_id, movie_id, gender, occupation, release_year, 
#          release_month, timestamp_month, age_group
categorical_exploration(ratings)

# Split ratings back into train and test ###############################
ratings_train <- merge(ratings_train, ratings, by=names(ratings_train))
ratings_test <- merge(ratings_test, ratings, by=names(ratings_test))

# TEST MODELS ###############################


