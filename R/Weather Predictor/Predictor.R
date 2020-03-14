suppressPackageStartupMessages(library(regtools))
suppressPackageStartupMessages(library(nnet))
suppressPackageStartupMessages(library('tidyr'))
#suppressPackageStartupMessages(library(corrplot))
data(day1)

relevantDataset <- day1[,c(3,5,9:13)] #Gets rid of useless data

training1 <- relevantDataset[101:nrow(relevantDataset), ]
test1 <- relevantDataset[1:100, ]

training2 <- relevantDataset[1:630, ]
test2 <- relevantDataset[631:nrow(relevantDataset), ]


############### INFERENCE FUNCTIONS ##########################################################
num_rows <- nrow(day1)

dummy_weathsit <- function(data) {
  sitDumms <- factorToDummies(as.factor(data$weathersit),'weathersit',omitLast=FALSE)
  return(sitDumms)
}

run_KNN <- function(training_set, k, pred_cols, input_pred, resp_col) {
  knnout <- basicKNN(as.matrix(training_set[pred_cols]),resp_col,input_pred,k)
  return(knnout)
}

predict_weathersit <- function(row_n) {
  actual <- day1[row_n,]$weathersit

  data_train <- day1[1:(num_rows-100),]
  mat <- dummy_weathsit(data_train)

  k <- 10
  pred_cols <- c('hum','windspeed','temp','atemp')

  input_pred <- as.numeric(day1[pred_cols][row_n,])

    knnout <- run_KNN(data_train, k, pred_cols, input_pred, mat)

    maxIdx <- 1
    maxSoFar <- 0.0
    idx <- 1
    for(elem in knnout$regests){
        if(as.numeric(elem) > maxSoFar){
            maxIdx <- idx
            maxSoFar <- as.numeric(elem)
        }
        idx <- idx + 1
    }
    if(maxIdx == actual){
        return(1)
    } else {
        return(0)
    }
}

ACC_weathersit <- function() {
  k <- 10
  tot_rows <- nrow(day1)
  vals <- c()

  for (row in (tot_rows-99):tot_rows) {
    vals <- c(vals,predict_weathersit(row))
  }

  print(paste("KNN Accuracy of prediction of weathersit: ", ((sum(vals)/length(vals))*100), "%"))
  cat("\n")
}

predict_atemp <- function(row_n){
    actual <- day1[row_n,]$atemp
    LM_Df <- day1[1:(num_rows-100),]

    LM_Summary <- glm(LM_Df$atemp ~ LM_Df$temp)

    LM_Coef <- LM_Summary$coefficients
    predicted <- as.numeric(LM_Coef[1]) + (as.numeric(LM_Coef[2])*day1[row_n,]$temp)

    error <- abs((actual-predicted)/actual)

    return(error)

}


ACC_atemp <- function(){
    all_errors <- c()
    for(row in ((nrow(day1) - 99)):nrow(day1)){
        all_errors <- c(all_errors, predict_atemp(row))
    }

    all_errors <- sort(all_errors)

    print(paste("LM Max Accuracy of prediction for atemp: ", (1 - min(all_errors))*100))
    print(paste("LM Median Accuracy of prediction for atemp: ", (1 - median(all_errors))*100))
    cat("\n")


}


############# USAGE INSTRUCTIONS AFTER FUNCTIONS ################################################ 

LM_Dataframe <- function(Dataset, columnRange, numDays, target){
    if(length(columnRange) > 1){
        colNames <- colnames(Dataset[, columnRange])
    } else {
        colNames <- names(Dataset)[columnRange[1]]
    }

    targetNo <- which(colnames(Dataset) == target)
    curr <- data.frame(Dataset[(numDays+1):(nrow(Dataset)), targetNo])
    colnames(curr)[1] <- target

    tempNum <- numDays 
    offset <- 1

    df1 <- data.frame(Dataset[tempNum:(nrow(Dataset) - offset), columnRange])
    counter1 <- 1
    for(coln in colNames){
        colnames(df1)[counter1] <- paste0(coln, as.character(tempNum))
        counter1 <- counter1 + 1
    }
    dfCols <- list(df1)
    

    tempNum <- tempNum - 1 
    offset <- offset + 1 

    while(tempNum > 0){

        df <- data.frame(Dataset[tempNum:(nrow(Dataset) - offset), columnRange])

        counter <- 1
        for(col in colNames){
            colnames(df)[counter] <- paste0(col,as.character(tempNum))
            counter <- counter + 1
        }

        dfCols[[offset]] <- df
        tempNum <- tempNum - 1
        offset <- offset + 1

    }

    combined <- do.call(cbind, dfCols)


    finalDf <- cbind(curr, combined)


    return(finalDf)

}



LM_Prediction <- function(Dataset, columnRange, numDays, target, operands, firstGo){
    LM_Df <- LM_Dataframe(Dataset, columnRange, numDays, target)



    LM_Summary <- lm(LM_Df)
    print(summary(LM_Summary))

    if(firstGo == 633){
        print(summary(LM_Summary))
    }

    LM_Coef <- LM_Summary$coefficients

    factorSum <- LM_Coef[1]
    for(idx in 2:length(LM_Coef)){
        factorSum <- factorSum + (LM_Coef[idx]*operands[idx-1])
    }

    return(factorSum)

}

# e.g. : 
# Dataset
# columnRange : c(0,1,2)
# numDays : num
# target : 'colName'
# features : c(predictor1val, predictor2val, predictor3val)
# multiclassRes : 1/0 flag
KNN_Prediction <- function(Dataset, columnRange,numDays, target, features,k, multiclassRes){

    KNN_Df <- LM_Dataframe(Dataset, columnRange, numDays, target)

    predictor_values <- KNN_Df[,2:ncol(KNN_Df)]

    training_values <- NULL

    if(multiclassRes == 0){
        training_values <- KNN_Df[,1]
    } else {
        training_values <- factorToDummies(as.factor(KNN_Df[, target]),target,omitLast=FALSE)
    }

    knnout <- basicKNN(predictor_values, training_values, features,k)

    if(multiclassRes== 0){
        return(knnout$regests)
    }else{
        maxIdx <- 1
        maxSoFar <- 0.0
        idx <- 1
        for(elem in knnout$regests){
            if(as.numeric(elem) > maxSoFar){
                maxIdx <- idx
                maxSoFar <- as.numeric(elem)
            }
            idx <- idx + 1
        }
        return(maxIdx)
    }
}


LM_Accuracy <- function(Dataset,testSet, columnRange, numDays, target){

    totalError <- c()
    totalTests <- 0

    colnames <- c()

    if(length(columnRange) > 1){
        colNames <- colnames(Dataset[, columnRange])
    } else {
        colNames <- names(Dataset)[columnRange[1]]
    }

     for(idx in 1:nrow(testSet)){
        operands <- c()
        for(day in 0:(numDays-1)){
            for(col in colNames){
                operands <- c(operands, testSet[idx + day, col])
            }
        }
        prediction <- LM_Prediction(Dataset, columnRange, numDays, target, operands, idx)
        actual <- testSet[idx + numDays, target]

        error  <- abs((actual-prediction)/actual)
        totalError <- c(totalError, error)
     }

     totalError <- sort(totalError)

     quantiles <- quantile(totalError)
     Q1 <- as.numeric(quantiles[2])
     Q3 <- as.numeric(quantiles[4])
     Iqr <- IQR(totalError)

     lowerFence <- Q1 - 1.5*Iqr 
     upperFence <- Q3 + 1.5*Iqr


     newError <- c()

    for(elem in totalError){
        if(elem < upperFence && elem > lowerFence){
            newError <- c(newError, elem)
        }
    }

    totalError <- newError

    cat("ACC: ", (mean(totalError))*100, "\n")
    
    return((mean(totalError))*100)
}


KNN_Accuracy <- function(Dataset,testSet, columnRange, numDays, target,k, multiclassRes){

    totalError <- c()
    totalTests <- 0

    colnames <- c()

    if(length(columnRange) > 1){
        colNames <- colnames(testSet[, columnRange])
    } else {
        colNames <- names(testSet)[columnRange[1]]
    }

     for(idx in 1:(nrow(testSet)-numDays)){
        features <- c()
        for(day in 0:(numDays-1)){
            for(col in colNames){
                features <- c(features, testSet[idx + day, col])
            }

        }

        prediction <- KNN_Prediction(Dataset, columnRange, numDays, target, features,k, multiclassRes)
        actual <- testSet[idx + numDays, target]

        if(is.na(prediction) || is.na(actual)){
            # print(prediction)
            # print(actual)
            print(nrow(testSet))
            print(idx)
            print(target)
            print(idx + numDays)
            print(testSet[idx,])
        }

        if(multiclassRes == 0) {
            error  <- abs((actual-prediction)/actual)
            totalError <- c(totalError, error)
        } else {
            if(prediction == actual){
                totalError <- c(totalError, 1)
            } else {
                totalError <- c(totalError, 0)
            }
        }
        
     }

     totalError <- sort(totalError)

     if(multiclassRes == 0){
        totalError <- sort(totalError)

        quantiles <- quantile(totalError)
        Q1 <- as.numeric(quantiles[2])
        Q3 <- as.numeric(quantiles[4])
        Iqr <- IQR(totalError)

        lowerFence <- Q1 - 1.5*Iqr 
        upperFence <- Q3 + 1.5*Iqr

        newError <- c()

        for(elem in totalError){
            if(elem < upperFence && elem > lowerFence){
                newError <- c(newError, elem)
            }
        }

        totalError <- newError
        cat("ACC: ", (mean(totalError))*100, "\n")
         return((mean(totalError))*100)
     } else {
         return((sum(totalError)/length(totalError))*100)
     }

}



################### USAGE ###################################

# For accuracy of LM: Use LM_Accuracy <- function(Dataset, columnRange, numDays, target)
#     where: Dataset <- relevantDataset
#            columnRange <- vector of columns you want to use as predictor_values
#            numDays <- how many days back you want to look 
#            target <- string value of the column of the Dataset you want to predict 
#     EXAMPLE: LM_Accuracy(relevantDataset, c(1,4), 1, "atemp") 

# For accuracy of KNN: Use KNN_Accuracy <- function(Dataset, columnRange, numDays, target,k, multiclassRes, multiclassPred, MC_Targets, MC_TargetWidths)
#     Where: Dataset <- relevantDataset
#            columnRange <- vector of columns you want to use as predictor_values
#            numDays <- how many days back you want to look
#            target <- string value of the column of the Dataset you want to predict
#            k <- how many nearest neighbors you want to look at for KNN 
#            multiclassRes <- takes 1 or 0 value. 1 if result to be predicted is categorical. 0 if not 
#            multiclassPred <- takes 1 or 0 value. 1 if any cateogrical variables are used as predicators. 0 if not. 
#            MC_Targets <- vector containing strings of names of categorical predictors, in the same order as they appear in the Datafame 
#            MC_TargetWidths <- number of values each categorical predictor can take. This is constant. Use: "list("season" = 4, "mnth" = 12, "weathersit" = 3)"
#     EXAMPLE: KNN_Accuracy(relevantDataset, c(1:7), 1, "weathersit", 2,1,1, c("season", "mnth", "weathersit"), list("season" = 4, "mnth" = 12, "weathersit" = 3))
#            Here, you are predicted the categorical varibale weathersit using all columns. Not that both multiclass arguments are 1 becuase youre using categorical predictors
#            to predict a categorical value. The vector of categorical predictors are in order. Only add the ones you are using to predict. 



powerset <- function(x) {
  sets <- lapply(1:(length(x)), function(i) combn(x, i, simplify = F))
  unlist(sets, recursive = F)
}


optimal_knnCat <- function(target){

    cat("Finding optimal KNN value for: ", target, "\n")

    predictors <- powerset(c(1:7))

    max <- 1000
    vals <- c("N/A")


    print(nrow(training1))
    print(nrow(test1))
    print(nrow(training2))
    print(nrow(test2))
    
    for(predictor in predictors){
        for(day in 1:3){
            for(neighbors in 2:4){
                curr1 <- KNN_Accuracy(training1,test1, predictor,day, target,neighbors,1) 
                curr2 <- KNN_Accuracy(training2, test2, predictor, day, target,neighbors, 1)
                curr <- (curr1 + curr2)/2
                if(curr < max){
                    vals <- c(predictor, day, neighbors)
                    max <- curr

                    print(vals)
                    print(max)
                }
            }
        }
    }

    print(max)
    print(vals)


}

optimal_knn <- function(target){

    cat("Finding optimal KNN value for: ", target, "\n")

    predictors <- powerset(c(1:7))

    max <- 1000
    vals <- c("N/A")
    
    for(predictor in predictors){
        for(day in 1:3){
            for(neighbors in 1:4){
                curr1 <- KNN_Accuracy(training1,test1, predictor,day, target,neighbors,0) 
                curr2 <- KNN_Accuracy(training2, test2, predictor, day, target,neighbors, 0)
                curr <- (curr1 + curr2)/2
                if(curr < max){
                    vals <- c(predictor, day, neighbors)
                    max <- curr
                }
            }
        }
    }

    print(max)
    print(vals)


}

optimal_lm <- function(target){

    cat("Finding optimal LM value for: ", target, "\n")

    predictors <- powerset(c(1:7))

    max <- 1000
    vals <- c("N/A")
    
    for(predictor in predictors){
        for(day in 1:3){
                curr1 <- LM_Accuracy(training1,test1, predictor, day, target)
                curr2 <- LM_Accuracy(training2, test2, predictor, day, target)
                curr <- (curr1 + curr2)/2
                if(curr < max){
                    vals <- c(predictor, day)
                    max <- curr
                }
        }
    }

    print(max)
    print(vals)


}

# optimal_lm("windspeed")
optimal_knnCat("weathersit")  

#c1 <- LM_Accuracy(training1, test1, c(1,4,7), 1, "windspeed")
#  c2 <- LM_Accuracy(training2, test2, c(3,4,7), 1, "temp")
#  print((c1+c2)/2)

#  c1 <- LM_Accuracy(training1, test1, c(3,5,7), 1, "temp")
#  c2 <- LM_Accuracy(training2, test2, c(3,5,7), 1, "temp")
#  print((c1+c2)/2)


#  curr1 <- KNN_Accuracy(training1,test1, c(4,1),1, "temp", 4, 0) 
#  curr2 <- KNN_Accuracy(training2, test2, c(4,1),1, "temp", 4, 0)
#  print((curr1+curr2)/2)


  
generate_predictors <- function(curr_pred,days,k,used_predictors,var){
  used_preds <- curr_pred
  new_predictors <- curr_pred
  num_pred <- length(curr_pred)
  
  used_nums <- c()
  new_num <- num_pred
  comb_of_predictors <- sum(rep(1,length(used_predictors))*(lengths(used_predictors) %in% used_nums))
  

  repeat{
    comb_of_predictors <- sum(rep(1,length(used_predictors))*(lengths(used_predictors) %in% used_nums))
    used_nums <- append(used_nums,new_num)
    new_num <- sample(c(1:7)[!c(1:7) %in% used_nums],1)
    if(length(used_nums) < 7 || comb_of_predictors %in% sum(choose(7,used_nums))){
      break
    }
  }
  num_pred <- new_num
  
  if(length(used_nums) == 7){
    gen_k <- sample(sample(2:25,curr_k),1)
    gen_days <- sample(sample(1:3,curr_days),1)
    return(list(gen_days,gen_k,unique(new_predictors)))
  }
  
  while(list(new_predictors) %in% used_predictors){
    #print(paste(c(new_predictors,"and the ",used_preds,"and the new",new_num,"and the not"),sep=''))
    if(length(used_preds) == 7){
      new_num <- new_num + sample(c(-1,-2,-3),1)
      if(new_num < 1){
        new_num <- 2
      }
    }
    if(0 %in% var$Acc){
      new_predictors <- sample(c(1:7),(new_num))
      break
    }
    
    sorted_preds <- sort(var$Acc[new_predictors])
    not_used_variables <- c(1:length(var$Feature))[!(c(1:length(var$Feature) %in% used_preds))]
    if(length(new_predictors) == new_num){
      
      new_predictors <- new_predictors[new_predictors != which(var$Acc == sorted_preds[length(sorted_preds)])]
      new_var_dex <- not_used_variables[which.min(var$Acc[not_used_variables])]
      
      new_predictors <- append(new_predictors,new_var_dex)
      used_preds <- unique(append(used_preds,new_predictors))
    }
    else if(length(new_predictors) > new_num){
      new_predictors <- c()
      for(pred in 1:new_num){
        new_pred <- which.min(var$Acc == sorted_preds[pred])
        new_predictors <- append(new_predictors,new_pred)
      }
      used_preds <- new_predictors
    }
    
    else{
      used_preds <- new_predictors
      for (pred in 1:(new_num-length(new_predictors))){
        new_dex <- not_used_variables[which.min(var$Acc[not_used_variables])]
        new_predictors <- append(new_predictors,new_dex)
        used_preds <- c(used_preds,new_dex)
        not_used_variables <- c(1:length(var$Feature))[!(c(1:length(var$Feature) %in% used_preds))]
      }
    }
  }
  curr_day<-days
  curr_k<-k
  return(list(curr_day,curr_k,unique(new_predictors)))
}

optimize_KNN <- function(dataset,target,max_accuracy,desired_accuracy,training_num,reward,ends=0,starting_values=list(),categroricals=c(1:3)){
  
  below_wanted <- data.frame()
  variables <- data.frame(Feature = colnames(dataset), Acc = 0, Times_Used = 0, Times_Under_Reward = 0)
  days <- data.frame(Days = c(1:7),Power = 0, Times_Used = 0)
  ks <- data.frame(Ks = c(1:25), Power = 0, Times_Used = 0)
  
  used_day <- c()
  used_k <- c()
  
  training <- c()
  test <- c()
  
  if(ends == 1){
    training <- data.frame(dataset[1:(nrow(dataset)-training_num), ])
    test <- data.frame(dataset[training_num+1:nrow(dataset), ])
  }else{
    training <- data.frame(dataset[(nrow(dataset)-training_num):nrow(dataset), ])
    test <- data.frame(dataset[1:(training_num-1), ])
  }
  
  # training <- dataset[101:nrow(relevantDataset), ]
  # test <- dataset[1:100, ]
  
  curr_days <- 1
  curr_k <- 2
  curr_predictors <- NULL
  
  if(length(starting_values == 3)){
    curr_predictors <- starting_values[1]
    curr_k <- starting_values[2]
    curr_days <- starting_values[3]
    curr_values
  }
  else{
    curr_predictors <- sample(1:length(variables$Feature),sample(1:4,1))
    curr_k <- sample(sample(2:25,curr_k),1)
    curr_days <- sample(sample(1:3,curr_days),1)
    curr_values <- list(curr_days,curr_k,curr_predictors)
  }

  best_accuracy <- max_accuracy
  previous_accuracy <- 0
  num_pred <- length(curr_predictors)
  
  used_predictor_comb <- list()
  curr_values <- list(curr_days,curr_k,curr_predictors)
  previous_values <- list()
  best_values <- curr_values
  reps <- 0
  
  while(best_accuracy >= desired_accuracy && reps < 250){
    
    pred_cat <- 0
    target_cat <- 0
    
    if((which(variables$Feature == target) %in% categroricals)){
      target_cat <- 1
    }
    
    new_accuracy <- KNN_Accuracy(training,test,curr_predictors,curr_days, target, curr_k,target_cat)
    
    #print(paste(c("Tried",curr_predictors, "looking back",curr_days," days and with ",curr_k, "neighbors", new_accuracy, "is compared to",best_accuracy," this is rep",reps),collapse = " "))
    print(reps)
    accuracy_diff <- new_accuracy-previous_accuracy 
    days$Power[which(days == curr_days)] <- (days$Power[which(days == curr_days)]+(new_accuracy/(length(curr_predictors)))*(10-length(curr_predictors))^(accuracy_diff/10))
    ks$Power[which(ks == curr_k)] <- (ks$Power[which(ks == curr_k)]+(new_accuracy/(length(curr_predictors))))
    
    
    
    if(length(previous_values) != 0){
      used_predictor_comb <- append(used_predictor_comb,list(curr_predictors))
    }
    
    accept_accuracy <- FALSE
    accuracy_diff <- new_accuracy-previous_accuracy
    
    if (new_accuracy < best_accuracy){
      best_values <- curr_values
      accept_accuracy <- TRUE
      previous_values <- curr_values
      curr_values <- generate_predictors(curr_predictors,curr_days,curr_k,used_predictor_comb,variables)
      curr_days <- curr_values[[1]]
      curr_k <- curr_values[[2]]
      curr_predictors <- curr_values[[3]]
    }
    else if(new_accuracy < max_accuracy){
      used_predictor_comb <- append(used_predictor_comb,curr_predictors)
      curr_predictors <- sample(1:length(variables$Feature),num_pred)
      curr_k <- sample(sample(2:25,curr_k),1)
      curr_days <- sample(sample(2:4,curr_days),1)
      previous_values <- curr_values
      curr_values <- list(curr_days,curr_k,curr_predictors)
    }
    else{
      previous_values <- curr_values
      curr_values <- generate_predictors(previous_values[[3]],curr_days,curr_k,used_predictor_comb,variables)
      curr_days <- curr_values[[1]]
      curr_k <- curr_values[[2]]
      curr_predictors<-curr_values[[3]]
    }
    
    if(previous_accuracy != 0){
      for (preds in 1:length(curr_predictors)){
        dex <- curr_predictors[preds]
        variables$Times_Used[dex] <- variables$Times_Used[dex] + 1
        # if (curr_predictors[preds] %in% best_values[[3]]){
        #   variables$Acc[dex] <- variables$Acc[dex]*((best_accuracy)/100)
        # }
        if(new_accuracy < reward){
          variables$Times_Under_Reward[dex] <-  variables$Times_Under_Reward[dex] + 1
          new_row <- data.frame(Predictors = curr_predictors,Acc = new_accuracy, Days = curr_days, Ks = curr_k)
        }
        variables$Acc[dex] <- ((variables$Acc[dex] + (new_accuracy/100)*(new_accuracy/length(curr_predictors)))*10^(accuracy_diff/10))/(variables$Times_Under_Reward[dex]+1)
      }
    }
    
    reps <- reps + 1
    if (reps > 1000000){
      break
    }
    if(accept_accuracy){
      previous_accuracy <- best_accuracy
      best_accuracy <- new_accuracy
    }
    else{
      previous_accuracy<-new_accuracy
    }
  }
  return(list(best_values,best_accuracy,below_wanted,variables,days,ks))
}

#optimize_KNN(relevantDataset, "season", 30, 15, 100, 23, 1)

# Optimize_KNN takes up to 9 parameters
# Dataset is the dataset to use (data_frame)
# Target is the variable to predict (as a string)
# Max_accuracy is the lowest acceptable accuracy (int)
# Desired_accuracy is the accuracy you want to get to (int)
# Training_num is how much of the dataset to use for training (int)
# Reward is an accuracy which if the value is low (good enough), you want to record it. (int)
# Ends decides which end of the dataset to take the training set from, beginning or end (0 for beg, 1 for end; the default is 0)
# Starting_Values are a list in the order list(days,k,c(predictors)), (default is empty and random values will be generated)
# Categoricals are which variables are categorical (Default is c(1,2,3))