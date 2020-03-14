remove_correlated_features <- function(ratings) {
  # comedy+drama are highly negatively correlated (remove comedy, drama stronger cor w/rating)
  # scifi+action are highly positively correlated (remove action, scifi stronger cor w/rating)
  # remove release_date (all info already addressed)
  # time_difference (highly correlated with release_year)
  # fantasy+childrens high correlation (remove fantasy)
  correleted_features <- c("comedy","action","release_date",
                           "fantasy","time_difference")
  ratings_no_correlated_features <- ratings[, !names(ratings) 
                                                  %in% correleted_features]
  return(ratings_no_correlated_features)
}