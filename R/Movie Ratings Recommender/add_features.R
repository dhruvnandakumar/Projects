add_features <- function(ratings) {
  # ADD TIME GROUPS
  ratings$release_year <- year(as_datetime(ratings$release_date))
  ratings$release_month <- month(as_datetime(ratings$release_date))
  ratings$timestamp_year <- year(as_datetime(ratings$timestamp))
  ratings$timestamp_month <- month(as_datetime(ratings$timestamp))
  ratings$time_difference <- as.period(as_datetime(ratings$timestamp)-
                              as_datetime(ratings$release_date)) %>% day

  # ADD AGE GROUPS
  age <- ratings %>% pull(age)
  ratings$age_group <- rep(0,nrow(ratings))
  ratings$age_group <- findInterval(age,c(10,20,30,40,50,60,70,80))
  ratings$age_group <- as.factor(ratings$age_group)
  levels(ratings$age_group) <- c("<10","10-20","20-30","30-40",
                                 "40-50","50-60","60-70","70+")
  
  # change type of release_month and timestamp_month the factor
  ratings$release_month <- as.factor(ratings$release_month)
  ratings$timestamp_month <- as.factor(ratings$timestamp_month)
  
  return(ratings)
}