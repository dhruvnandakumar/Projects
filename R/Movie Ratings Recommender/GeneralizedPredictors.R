LM_Accuracy <- function(trainingSet, testSet, predictWith){

    predictWithR <- c(predictWith, "rating")

    trainingFrame <- trainingSet[predictWithR] #Create training dataFrame
    predictionFrame <- testSet[predictWithR]
    

    predictionResults <- predictionFrame[,c("rating")]#Vector of true values to test accuracy against 
    predictionFrame <- predictionFrame[predictWith] #Create predictions dataFrame 

    lm_model <- lm(rating~. , data=trainingFrame) #Create the model

    # print(summary(lm_model)) #Debug

    predictions <- predict(lm_model,predictionFrame) #Predict with model
    

    totalError <- c()


    vals <- abs((predictionResults - predictions)/predictionResults)
    
    MAPE <- (mean(vals)*100)

    return(MAPE)


}

MF_Accuracy <- function(trainingSet, testSet, rnk=10){

    #Training and test sets ordered usrId, movieNum, Rating

    #Make lookup tables -- May not be necessary
    # originalUsrID <- as.character(unique(trainingSet[,1]))
    # originalMovieNum <- as.character(unique(trainingSet[,2]))
    # consecuitveUsers <- (1:length(originalUsrID))
    # names(consecuitveUsers) <- originalUsrID
    # consecuitveMovies <- (1:length(originalMovieNum))
    # names(consecuitveMovies) <- originalMovieNum

    # # Replace with consecutive values -- Also may not be necessary 
    # trainingSet[,1] <- consecuitveUsers[as.character(trainingSet[,1])]
    # trainingSet[,2] <- consecuitveMovies[as.character(trainingSet[,2])]

    require(recosystem) #Make sure TA's computer has recosystem package included 

    recommenderObject <- Reco()

    recommenderTrainer <- data_memory(trainingSet[,1], trainingSet[,2], trainingSet[,3],index1=TRUE) #Train the model 

    recommenderTest <- data_memory(testSet[,1],testSet[,2],index1=TRUE)

    recommenderObject$train(recommenderTrainer,opts = c(dim=rnk, nmf=TRUE,nthread=4,verbose=FALSE))

    predictedVals <- recommenderObject$predict(recommenderTest,out_memory())

    actualVals <- testSet[,3]

    diff <- abs((actualVals-predictedVals)/actualVals)

    MAPE <- (mean(diff)*100)

    return(MAPE)
}
