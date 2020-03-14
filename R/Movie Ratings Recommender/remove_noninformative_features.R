remove_noninformative_features <- function(ratings) {
  # remove visually identified features that have very low correlation with rating
  no_correlation <- c('timestamp', 'unknown', 'adventure','animation',
                      'musical','thriller','western')
  ratings_no_correlation <- ratings[, !names(ratings) %in% no_correlation]
  return(ratings_no_correlation)
}