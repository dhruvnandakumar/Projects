# Exploring the MovieLens dataset and prediction methods

## Preprocessing
1. Use `ua.base` and `ua.test` as premade train/test split.
    * Useful because the test set has exactly 10 ratings from each user, so it
      gives a fair evaluation of prediction accuracy.
    * Combine sets for easier preprocessing, then re-split after.
2. Add the movie information into the ratings dataframe.
3. Convert the `$release_date` column from a string into seconds from UNIX epoch,
   the same format as `$timestamp` from the rating data. This way it can be
   treated as quantitative data.
    * Running into the issue that some movies were released before the UNIX epoch
      in 1970 being converted to negative values (presumably why the dates are
      formatted as strings).
        * To keep values positive, create custom epoch at the earliest timestamp
          (1922) and convert seconds to days to prevent overflow.
    * Summary of the columns after preprocessing:

        Column Name     | Minumum | 1st Quartile | Median | Mean  | 3rd Quartile | Maximum | NA's
        --------------- | ------- | ------------ | ------ | ----- | ------------ | ------- | ----
        `$release_date` | 0       | 23376        | 26298  | 24145 | 27299        | 28054   | 9
        `$timestamp`    | 27656   | 27711        | 27750  | 27758 | 27813        | 27871   | 0

    * Looking at the maximums for each column, it appears that the latest
      release date is after the latest rating timestamp. Upon further analysis,
      there are a handful of ratings that ocurred before the movie's release
      date, as much as 292 days before.
