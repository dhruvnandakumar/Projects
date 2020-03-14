# EXPLORE: gender, occupation, release_month, timestamp_month, age_group
# user_id, movie_id,  (include in heatmap!!)

categorical_exploration <- function(ratings) {
  ##### GENDER #########################################################
  # Distribution of Gender
  p1 <- ggplot(ratings, aes(x = factor(gender), fill = factor(gender))) +
    geom_bar( show.legend=FALSE) +
    xlab("Gender") +
    ylab("Total Count") +
    ggtitle("Distribution of Gender")
  
  # Ratings by Gender
  p2 <- ggplot(subset(ratings, !is.na(gender)), aes(x = gender, fill = as.factor(rating))) +
    geom_bar() +
    ggtitle("Rating Count by Gender") +
    xlab("Gender") +
    ylab("Total Count") +
    labs(fill = "Survived")
  
  # Average rating by gender
  male_mean <- ratings %>% filter(gender=='M') %>% pull(rating) %>% mean
  female_mean <- ratings %>% filter(gender=='F') %>% pull(rating) %>% mean
  mean_gender <- c(male_mean, female_mean)
  gender <- c("male","female")
  mean_gender_df <- data.frame(gender, mean_gender)
  p3 <- ggplot(mean_gender_df, aes(x=gender, y=mean_gender)) + 
    geom_bar(stat="identity") +
    ggtitle("Average Rating by Gender") +
    xlab("Gender") +
    ylab("Average Rating")
  
  ##### OCCUPATION
  # Distribution of Gender
  p4 <- ggplot(ratings, aes(x = factor(gender), fill = factor(gender))) +
    geom_bar( show.legend=FALSE) +
    xlab("Gender") +
    ylab("Total Count") +
    ggtitle("Distribution of Gender")
  
  # Ratings by Gender
  p5 <- ggplot(subset(ratings, !is.na(gender)), aes(x = gender, fill = as.factor(rating))) +
    geom_bar() +
    ggtitle("Rating Count by Gender") +
    xlab("Gender") +
    ylab("Total Count") +
    labs(fill = "Survived")
  
  # Average rating by gender
  male_mean <- ratings %>% filter(gender=='M') %>% pull(rating) %>% mean
  female_mean <- ratings %>% filter(gender=='F') %>% pull(rating) %>% mean
  mean_gender <- c(male_mean, female_mean)
  gender <- c("male","female")
  mean_gender_df <- data.frame(gender, mean_gender)
  p6 <- ggplot(mean_gender_df, aes(x=gender, y=mean_gender)) + 
    geom_bar(stat="identity") +
    ggtitle("Average Rating by Gender") +
    xlab("Gender") +
    ylab("Average Rating")
  ##############################################3
  
  ##### AGE_GROUP
  p7 <- ggplot(ratings, aes(x = age_group, fill = as.factor(rating))) +
    geom_bar() +
    ggtitle("Rating Count by Age") +
    xlab("Age") +
    ylab("Total Count") +
    labs(fill = "Rating")
  
  
  ##### Release Month
  p8 <- ggplot(ratings, aes(x = release_month, fill = as.factor(rating))) +
    geom_bar() +
    ggtitle("Rating Count by Release Month") +
    xlab("Age") +
    ylab("Total Count") +
    labs(fill = "Rating")
  
  ##### Timestamp Month
  p9<- ggplot(ratings, aes(x = timestamp_month, fill = as.factor(rating))) +
    geom_bar() +
    ggtitle("Rating Count by Timestamp Month") +
    xlab("Age") +
    ylab("Total Count") +
    labs(fill = "Rating")
  
}





