#install a library
#install.packages("RSQLite")
#'imports
library("RSQLite")#setup database connection
#library("nlme") #lme
library("tidyr") #convert from long to wide data format and back
library("DT")#too show colored NE matrix
library("mosaic")#favstats
library("ggplot2")#beanplot
library("rPref") #psel, pareto_optimal
library("coin")#for wilcox_test
library("BSDA")#sign test, wilcox alternative
library("Matching")#bootstrap Kolmogorov-Smirnov test
library("tidyverse")#for str_replace
library("arrangements")#make all combinations of players
#http://www.cookbook-r.com/Manipulating_data/Converting_data_between_wide_and_long_format/
#library("multcomp")#for the multiple comparison stuff 
#library("ez")
# connect to the sqlite file
#setwd("D:/Onedrive/master/thesis/Qt/text_finder_example/test")#set work directory



#read a table from the database and average over rounds & iterations
read_file <-function(con,tab,compdata)
{
  str1="SELECT DISTINCT pl_id0,pl_id1 FROM "
  querry <- paste(str1,tab,";")
  resultgame <- dbGetQuery(con, querry)
  resultgame<- cbind(resultgame,rewf_pl0=0,rewf_pl1=0,rewl_pl0=0,rewl_pl1=0)
  
  for(row in 1:nrow(resultgame)) 
  {
    for(r in 0:(compdata$rounds-1))#for all rounds
    {
      str1="SELECT sum(rew_pl0),sum(rew_pl1) FROM "
      str2=" WHERE round ="
      str3=" AND rec_FL = 'F' AND pl_id0 = '"
      str4= "' AND pl_id1 = '"
      querry <- paste(str1,tab,str2,r,str3,resultgame$pl_id0[row],str4,resultgame$pl_id1[row],"';",sep="")
      #print(querry)
      rounddata <- dbGetQuery(con, querry)
      resultgame$rewf_pl0[row]<-resultgame$rewf_pl0[row] + rounddata[1,1]/(compdata$rounds*compdata$record_f)
      resultgame$rewf_pl1[row]<-resultgame$rewf_pl1[row] + rounddata[1,2]/(compdata$rounds*compdata$record_f)
      
      str3=" AND rec_FL = 'L' AND pl_id0 = '"
      querry <- paste(str1,tab,str2,r,str3,resultgame$pl_id0[row],str4,resultgame$pl_id1[row],"';",sep="")
      rounddata <- dbGetQuery(con, querry)
      resultgame$rewl_pl0[row]<-resultgame$rewl_pl0[row] + rounddata[1,1]/(compdata$rounds*compdata$record_l)
      resultgame$rewl_pl1[row]<-resultgame$rewl_pl1[row] + rounddata[1,2]/(compdata$rounds*compdata$record_l)
    }
  }
  resultgame$rewf_pl0[is.na(resultgame$rewf_pl0)] <- 0 #remove the na values we might get if cells are empty
  resultgame$rewf_pl1[is.na(resultgame$rewf_pl1)] <- 0
  resultgame$rewl_pl0[is.na(resultgame$rewl_pl0)] <- 0
  resultgame$rewl_pl1[is.na(resultgame$rewl_pl1)] <- 0
  
  #symmetric games fix
  if(length(players)^2 !=length(resultgame$pl_id0))
  {
    print("symmetric game")
    df_other_half<- resultgame[(resultgame$pl_id0!=resultgame$pl_id1),]
    colnames(df_other_half) <- c("pl_id1","pl_id0","rewf_pl1","rewf_pl0","rewl_pl1","rewl_pl0")
    resultgame<-rbind(resultgame,df_other_half)
    #rbind(
    #  data.frame(c(resultgame, sapply(setdiff(names(df_other_half), names(resultgame)), function(x) NA))),
    #  data.frame(c(df_other_half, sapply(setdiff(names(resultgame), names(df_other_half)), function(x) NA))))
  }
  return(resultgame)
}


#creates the result dataframe with first, last recorded rounds or both averages
average_f_l_rounds<-function(data,first,last)
{
  if(first==TRUE)
  {
    res<-data[c("pl_id0","rewf_pl0","pl_id1","rewf_pl1")]
    names(res) <- c("pl_id0", "rew_pl0","pl_id1","rew_pl1")
  } 
  if(last==TRUE && first==FALSE)
  {
    res<-data[c("pl_id0","rewl_pl0","pl_id1","rewl_pl1")]
    names(res) <- c("pl_id0", "rew_pl0","pl_id1","rew_pl1")
  } 
  if(last==TRUE && first==TRUE){#add last to first and devide by 2
    res$rew_pl0 <- (res$rew_pl0 + data$rewl_pl0)/2
    res$rew_pl1 <- (res$rew_pl1 + data$rewl_pl1)/2
  }
  return(res)
}

#if only column players(actuator) rewards are used only_col_rew==TRUE or not:only_col_rew=FALSE. 
average_row_col<-function(data,average)
{
  if(average==FALSE)
  {
    data$rew_fin<-data$rew_pl1
  }
  else
  {
    i=1
    while( i <= nrow(data))
    {
      if(data$pl_id0[i]==data$pl_id1[i])#plo == pl1
      {
        data$rew_fin[i]<-(data$rew_pl0[i]+data$rew_pl1[i])/2 #average both
      }
      if(data$pl_id0[i]!=data$pl_id1[i])#plo != pl1
      {
        row<-data[ (data$pl_id0 == data$pl_id1[i])&(data$pl_id1 == data$pl_id0[i]) , ]
        data$rew_fin[i]<-(row$rew_pl0[1]+data$rew_pl1[i])/2 #average both
  #      print(row$rew_pl0[1])
        #print((row$rew_pl0[1]+data$rew_pl1[i])/2)
      }
      i=i+1
    }
  }
  return(data)
}

average_regwgr_rc<-function(data,players)
{
  #symmetric games fix
  if(length(players)^2 !=length(data$pl_id0))
  {
    print("symmetric game")
    df_other_half<- data[(data$pl_id0!=data$pl_id1),]
    colnames(df_other_half) <- c("pl_id1","pl_id0","rew_pl1","rew_pl0","regr_pl1","regr_pl0")
    data<-rbind(data,df_other_half)
  }
  #print(data)
  #average row, col
  i=1
  while( i <= nrow(data))
  {
    if(data$pl_id0[i]==data$pl_id1[i])#plo == pl1
    {
      data$rew_fin[i]<-(data$rew_pl0[i]+data$rew_pl1[i])/2 #average both
      data$regr_fin[i]<-(data$regr_pl0[i]+data$regr_pl1[i])/2 #average both
    }
    if(data$pl_id0[i]!=data$pl_id1[i])#plo != pl1
    {
      row<-data[ (data$pl_id0 == data$pl_id1[i])&(data$pl_id1 == data$pl_id0[i]) , ]
      data$rew_fin[i]<-(row$rew_pl0[1]+data$rew_pl1[i])/2 #average both
      data$regr_fin[i]<-(row$regr_pl0[1]+data$regr_pl1[i])/2 #average both
      #      print(row$rew_pl0[1])
      #print((row$rew_pl0[1]+data$rew_pl1[i])/2)
    }
    i=i+1
  }
  
  return(data)
}



# ------------------------------------------------------------------------------------------
# p. 247 of Statistical Analysis with R for Dummies by Joseph Schmuller
# chapter: Repeated measures ANOVA in R  
#anova example
Vb_ANOVA<-function()
{
  method1.scores <- c(95,91,89,90,99,88,96,98,95) 
  method2.scores <- c(83,89,85,89,81,89,90,82,84,80) 
  method3.scores <- c(68,75,79,74,75,81,73,77)
  Score <- c(method1.scores, method2.scores, method3.scores)
  Method <- rep(c("method1", "method2", "method3"), times=c(length(method1.scores),  length(method2.scores), length(method3.scores)))
  Training.frame <- data.frame(Method,Score)
  Training.frame
  Method
  analysis <-aov(Score ~ Method,data = Training.frame)
  summary(analysis)
  boxplot(Score ~ Method,data = Training.frame) 
}


#plot the data  
plot_data<-function(data.pross,matrix)
{
  
  plot.new()
  x <- seq(0, 100, length=1000)
  #normal distribution, the matrix means is not exactly 50, sd is also ofset
  ourdistr=150*dnorm(x, mean=mean(data.pross$rew, sd=sd(data.pross$rew)))
  gamedistr=150*dnorm(x, mean=mean(matrix), sd=sd(matrix))
  #  norm_distr_sc=norm_distr*100
  
  hist(data.pross$rew,col=rgb(1,0,0,1/4),xlim=c(0,100),main="Histogram for reward frequency",breaks =20,xlab="reward")
  hist(matrix,col=rgb(0,1,0,1/4),xlim=c(0,100),add=T,breaks =20)
  #curve(dnorm(x, mean=mean(data$rew_fin), sd=sd(data$rew_fin)), y = 0, to = 150, add=T, col="blue")  
  lines(x,ourdistr,col="red",lwd=4)
  lines(x,gamedistr,col="green",lwd=4)
  #  lines(x,norm_distrib,col="blue",lwd=4)
  axis(1, at=seq(0, 100, by=10), labels=seq(0, 100, by=10))
  axis(1, at=seq(0, 100, by=5), labels=FALSE)
  #qqplot 
  #  qqnorm(data$rew)
  #  qqline(data$rew)
  #https://www.dummies.com/programming/r/how-to-use-quantile-plots-to-check-data-normality-in-r/
}


#create accumulated payoff plots and store in image_loc 
#analyse_FL='L'
ACP_plots <-function(con,tab,compdata,analyse_FL)
{
  str1="SELECT DISTINCT pl_id0,pl_id1 FROM "
  querry <- paste(str1,tab,";")
  resultgame <- dbGetQuery(con, querry)
  #resultgame<- cbind(resultgame,rewf_pl0=0,rewf_pl1=0,rewl_pl0=0,rewl_pl1=0)
  
  matrix<-as.integer(unlist(strsplit(compdata$matrix, "[,]")))
  #compute average rewards
  Avr_rewpl0<-mean(matrix[is.odd(seq_along(matrix))]) 
  Avr_rewpl1<-mean(matrix[is.even(seq_along(matrix))]) 
  
  #compute maxmin value
  pl0.act<-rep(rep(1:compdata$game_a),compdata$game_a)
  pl1.act<-rep(1:compdata$game_a, each=compdata$game_a)

  matrix<-as.integer(unlist(strsplit(compdata$matrix, "[,]")))
  game.mat = data.frame(pl0.act,pl1.act)
  game.mat$rewpl0<-matrix[is.odd(seq_along(matrix))] 
  game.mat$rewpl1<-matrix[is.even(seq_along(matrix))] 
  maxmin_pl0=0
  maxmin_pl1=0 
  
  #game.mat
  for(act in (0:(compdata$game_a-1)))#for all actions
  {
    #pl0
    act_ind<-game.mat$pl0.act==(act+1)
    min_act=min(game.mat$rewpl0[act_ind])
    if(min_act>maxmin_pl0)maxmin_pl0=min_act
    #pl1
    act_ind<-game.mat$pl1.act==(act+1)
    min_act=min(game.mat$rewpl1[act_ind])
    if(min_act>maxmin_pl1)maxmin_pl1=min_act  
  }
  if(analyse_FL=="L")#last
  {
    cum_maxmin_0<-cumsum(rep(maxmin_pl0,compdata$record_l))
    cum_maxmin_1<-cumsum(rep(maxmin_pl1,compdata$record_l))                    
  }
  else#first
  {
    cum_maxmin_0<-cumsum(rep(maxmin_pl0,compdata$record_f))
    cum_maxmin_1<-cumsum(rep(maxmin_pl1,compdata$record_f))    
  }

  ks_results<-data.frame(A=character(),B=character(),KS_pvalue=double(),KS_boot_pvalue=double(),conv_pl0=character(),conv_pl1=character(),pdom_pl0=character(),pdom_pl1=character(),
                         stringsAsFactors=FALSE)
  for(row in 1:nrow(resultgame)) 
  {
    if(analyse_FL=="L"){round_average= data.frame(matrix(vector(), compdata$record_l, 2, dimnames=list(c(), c("avr_pl0", "avr_pl1"))),stringsAsFactors=F)}
    else{round_average= data.frame(matrix(vector(), compdata$record_f, 2, dimnames=list(c(), c("avr_pl0", "avr_pl1"))),stringsAsFactors=F)}
    for(r in 0:(compdata$rounds-1))#for all rounds
    {
      str1="SELECT rew_pl0,rew_pl1 FROM "
      str2=" WHERE round = "
      str3=" AND rec_FL = '"
      str4 ="' AND pl_id0 = '"
      str5= "' AND pl_id1 = '"
      querry <- paste(str1,tab,str2,r,str3,analyse_FL,str4,resultgame$pl_id0[row],str5,resultgame$pl_id1[row],"';",sep="")
      #print(querry)
      rounddata <- dbGetQuery(con, querry)
      
      #plot(ecdf(rounddata$rew_pl1))#, main= )
      cum_pl0<-cumsum(rounddata$rew_pl0)
      cum_pl1<-cumsum(rounddata$rew_pl1)
      
      if(identical(cum_pl0,cum_pl1)==FALSE) 
      {
        ks_res=ks.test(cum_pl0,cum_pl1)#,simulate.p.value=TRUE, B=1000, alternative="g"
        #print(ks_res$p.value)
        ks_boot_res=ks.boot(cum_pl0,cum_pl1, nboots=1000)#, alternative="g"
        #print(ks_boot_res$ks.boot.pvalue)
        #print(paste())
        toString(resultgame$pl_id0[row])

        #convergence check for dataframe
        mid0<-cum_pl0[length(cum_pl0)/2]
        end0<-cum_pl0[length(cum_pl0)]
        mid1<-cum_pl1[length(cum_pl1)/2]
        end1<-cum_pl1[length(cum_pl1)]
        if(abs(end0-2*mid0)<Avr_rewpl0)conv_pl0<-"y"
        else conv_pl0<-"n"
        if(abs(end1-2*mid1)<Avr_rewpl1)conv_pl1<-"y"
        else conv_pl1<-"n"
        #prob domination
        pdom0<-cum_pl0>cum_pl1
        if(all(pdom0==TRUE)) pdom_pl0<-"y"
        else pdom_pl0<-"n"
        pdom1<-cum_pl1>cum_pl0
        if(all(pdom1==TRUE)) pdom_pl1<-"y"
        else pdom_pl1<-"n"          
          
        resrow<-data.frame(toString(resultgame$pl_id0[row]),toString(resultgame$pl_id1[row]),ks_res$p.value,ks_boot_res$ks.boot.pvalue,conv_pl0,conv_pl1,pdom_pl0,pdom_pl1)
        names(resrow)<-c("A","B","KS_pvalue","KS_boot_pvalue","conv_pl0","conv_pl1","pdom_pl0","pdom_pl1")
        ks_results<-rbind(ks_results,resrow)
      }
      else{print(paste("aa",resultgame$pl_id0[row],resultgame$pl_id1[row]))}

      One_ACP_plot(r,nrow(rounddata),resultgame$pl_id0[row],resultgame$pl_id1[row],cum_pl0,cum_pl1,cum_maxmin_0,cum_maxmin_1,Avr_rewpl0,Avr_rewpl1)
      if(r==0)#averages
      {
        round_average$avr_pl0=(rounddata$rew_pl0/compdata$rounds)
        round_average$avr_pl1=(rounddata$rew_pl1/compdata$rounds)
      }
      else
      {
        round_average$avr_pl0=round_average$avr_pl0+(rounddata$rew_pl0/compdata$rounds)
        round_average$avr_pl1=round_average$avr_pl1+(rounddata$rew_pl1/compdata$rounds)
      }
    }
    cum_avrpl0<-cumsum(round_average$avr_pl0)
    cum_avrpl1<-cumsum(round_average$avr_pl1)
    One_ACP_plot(-1,nrow(round_average),resultgame$pl_id0[row],resultgame$pl_id1[row],cum_avrpl0,cum_avrpl1,cum_maxmin_0,cum_maxmin_1,Avr_rewpl0,Avr_rewpl1)
  }
  #print(Avr_rewpl0)
  #print(Avr_rewpl1)
  print(ks_results)
  #print(loc_images)
  #ecdf(rounddata$rew_pl1)
  #cumsum(rounddata$rew_pl1)
}

One_ACP_plot<-function(round,iterations,pl0_name,pl1_name,cum_pl0,cum_pl1,cum_maxmin_0,cum_maxmin_1,Avr_rewpl0,Avr_rewpl1)
{
  if(round==-1)#averaging over all rounds
  {
    Title<-paste("Average Accumulated payoff: "," pl0(",pl0_name,") vs pl1(",pl1_name,")",sep="")
    filename=paste(loc_images,"AVR ACP ",pl0_name," ",pl1_name,".png",sep="")
  }
  else 
  {
    Title<-paste("Accumulated payoff round:",round," pl0(",pl0_name,") vs pl1(",pl1_name,")",sep="")
    filename=paste(loc_images,"ACP ",pl0_name," ",pl1_name," ",round,".png",sep="")
  }
  png(filename, 490, 350)
  plot(1:iterations, cum_pl0,ylim=c(0,max(cum_pl0,cum_pl1)) ,col="red" ,main=Title,xlab="Recorded round", ylab="Reward")
  lines(1:iterations,cum_pl0, col="dark red" )# join the points 
  lines(1:iterations, cum_maxmin_0, col="red",lty=2)#maxmin line
  points(1:iterations, cum_pl1, col="green" )
  lines(1:iterations, cum_pl1, col="green" )
  lines(1:iterations, cum_maxmin_1, col="#00FF00",lty=2)#maxmin line,line colored
  
  legend(1, max(cum_pl0,cum_pl1), legend=c(paste("pl0",pl0_name), paste("pl1",pl1_name),"maxmin pl0","maxmin pl1"),
         col=c("dark red","green","red","#00FF00"), lty=c(1,1,2,2), cex=0.8)
  
  #check if converged: rew at end if double half way reward
  mid0<-cum_pl0[length(cum_pl0)/2]
  end0<-cum_pl0[length(cum_pl0)]
  mid1<-cum_pl1[length(cum_pl1)/2]
  end1<-cum_pl1[length(cum_pl1)]
  if(abs(end0-2*mid0)<Avr_rewpl0)text((iterations/2), max(cum_pl0,cum_pl1), paste("pl0:",mid0,":",end0),cex = .8,col= "green")
  else text(x=(iterations/2), y=max(cum_pl0,cum_pl1), paste("pl0:",mid0,":",end0),cex = .8,col= "red")
  
  if(abs(end1-2*mid1)<Avr_rewpl1){text(x=(iterations/2), y=max(cum_pl0,cum_pl1)-max(cum_pl0,cum_pl1)/20, paste("pl1:",mid1,":",end1),cex = .8,col= "green")}
  else{text(x=(iterations/2), y=max(cum_pl0,cum_pl1)-max(cum_pl0,cum_pl1)/20, paste("pl1:",mid1,":",end1),cex = .8,col= "red")}
  dev.off()#save
}

#ACP_plots(con,"compdata2",compdata,'L')#F, firs rounds, L, last rounds. 
#Regret_plots(con,"compdata2",compdata,'L')


#regret & ACP hulp function: get odd/even indices
is.odd <- function(x) x %% 2 != 0
is.even <- function(x) x %% 2 == 0
#create regret plots. 
Regret_plots <-function(con,tab,compdata,analyse_FL)
{
  #analyse_FL='L'
  matrix<-as.integer(unlist(strsplit(compdata$matrix, "[,]")))
  #regret calculation
  pl0.act<-rep(rep(1:compdata$game_a),compdata$game_a)
  pl1.act<-rep(1:compdata$game_a, each=compdata$game_a)
  
  reg.act = data.frame(pl0.act,pl1.act)
  #reg.act
  reg.act$rewpl0<-matrix[is.odd(seq_along(matrix))] 
  reg.act$rewpl1<-matrix[is.even(seq_along(matrix))] 

  str1="SELECT DISTINCT pl_id0,pl_id1 FROM "
  querry <- paste(str1,tab,";")
  resultgame <- dbGetQuery(con, querry)
  
  average_ret_vals<-data.frame(matrix(vector(),0, 6, dimnames=list(c(), c("pl_id0","pl_id1","rew_pl0","rew_pl1","regr_pl0", "regr_pl1"))),stringsAsFactors=F)
  
  for(row in 1:nrow(resultgame)) 
  {
    #row=1
    if(analyse_FL=="L"){round_average= data.frame(matrix(vector(), compdata$record_l, 4, dimnames=list(c(), c("avrew_pl0", "avrew_pl1","avregr_pl0", "avregr_pl0"))),stringsAsFactors=F)}
    else{round_average= data.frame(matrix(vector(), compdata$record_f, 4, dimnames=list(c(), c("avrew_pl0", "avrew_pl1","avregr_pl0", "avregr_pl0"))),stringsAsFactors=F)}
    for(r in 0:(compdata$rounds-1))#for all rounds
    {
      #r=1
      str1="SELECT act_pl0,rew_pl0,act_pl1,rew_pl1 FROM "
      str2=" WHERE round = "
      str3=" AND rec_FL = '"
      str4 ="' AND pl_id0 = '"
      str5= "' AND pl_id1 = '"
      querry <- paste(str1,tab,str2,r,str3,analyse_FL,str4,resultgame$pl_id0[row],str5,resultgame$pl_id1[row],"';",sep="")
      #print(querry)
      rounddata <- dbGetQuery(con, querry)
      
      #make empty colums
      rounddata$reg_pl0<-0
      rounddata$reg_pl1<-0
      
      #compute regret
      for(act_ind in (0:(compdata$game_a-1)))#for all actions
      {
        #act_ind=1
        #pl0
        index<- rounddata$act_pl1==act_ind
        reg_ind<-reg.act$pl1.act==(act_ind+1)
        rounddata$reg_pl0[index] <-max(reg.act$rewpl0[reg_ind])-(rounddata$rew_pl0[index])#sum(reg_ind, na.rm = TRUE) *
        #pl1
        index<- rounddata$act_pl0==act_ind
        reg_ind<-reg.act$pl0.act==(act_ind+1)
        rounddata$reg_pl1[index] <-max(reg.act$rewpl1[reg_ind])-(rounddata$rew_pl1[index])#sum(reg_ind, na.rm = TRUE))# *
      }
      cum_pl0<-cumsum(rounddata$reg_pl0)
      cum_pl1<-cumsum(rounddata$reg_pl1)
      #makes a plot of one round
      #One_regr_plot(r,nrow(rounddata),resultgame$pl_id0[row],resultgame$pl_id1[row],cum_pl0,cum_pl1,tab)
      if(r==0)#averages
      {
        round_average$avrew_pl0=(rounddata$rew_pl0/compdata$rounds)
        round_average$avrew_pl1=(rounddata$rew_pl1/compdata$rounds)
        round_average$avregr_pl0=(rounddata$reg_pl0/compdata$rounds)
        round_average$avregr_pl1=(rounddata$reg_pl1/compdata$rounds)
      }
      else
      {
        round_average$avrew_pl0=round_average$avrew_pl0+(rounddata$rew_pl0/compdata$rounds)
        round_average$avrew_pl1=round_average$avrew_pl1+(rounddata$rew_pl1/compdata$rounds)
        round_average$avregr_pl0=round_average$avregr_pl0+(rounddata$reg_pl0/compdata$rounds)
        round_average$avregr_pl1=round_average$avregr_pl1+(rounddata$reg_pl1/compdata$rounds)
      }
    }
    cum_avrpl0<-cumsum(round_average$avregr_pl0)
    cum_avrpl1<-cumsum(round_average$avregr_pl1)
    One_regr_plot(-1,nrow(round_average),resultgame$pl_id0[row],resultgame$pl_id1[row],cum_avrpl0,cum_avrpl1,tab)
    datarow<-data.frame(pl_id0=resultgame$pl_id0[row],pl_id1=resultgame$pl_id1[row],rew_pl0=mean(round_average$avrew_pl0),rew_pl1=mean(round_average$avrew_pl1),regr_pl0=mean(round_average$avregr_pl0),regr_pl1=mean(round_average$avregr_pl1),stringsAsFactors=F)
    average_ret_vals<-rbind(average_ret_vals,datarow)
  }
  return(average_ret_vals) 
}

One_regr_plot<-function(round,iterations,pl0_name,pl1_name,cum_pl0,cum_pl1,tab)
{
  if(round==-1)#averaging over all rounds
  {
    Title<-paste("Average Accumulated regret: "," pl0(",pl0_name,") vs pl1(",pl1_name,")",sep="")
    filename=paste(loc_images,"AVR REGRET ",tab,pl0_name," ",pl1_name,".png",sep="")
  }
  else 
  {
    Title<-paste("Acumulated regret round:",round," pl0(",pl0_name,") vs pl1(",pl1_name,")",sep="")
    filename=paste(loc_images,"REGRET ",tab,pl0_name," ",pl1_name," ",round,".png",sep="")
  }
  
  png(filename, 490, 350)
  plot(1:iterations, cum_pl0,ylim=c(min(cum_pl0,cum_pl1),max(cum_pl0,cum_pl1)) ,col="red" ,main=Title,xlab="Recorded round", ylab="Regret")
  lines(1:iterations,cum_pl0, col="red" )# join the points 
  points(1:iterations, cum_pl1, col="green" )
  lines(1:iterations, cum_pl1, col="green" )
  legend(1, y=(max(cum_pl0,cum_pl1,1)-max(cum_pl0,cum_pl1,1)/20), legend=c(paste("pl0",pl0_name) , paste("pl1",pl1_name)),
         col=c("red", "green"), lty=1:2, cex=0.8)
  dev.off()  
}



#call gambit and show the returned matrix
Gambit_NE<-function(players,data.raw)
{
  data.raw <- data.raw[order(data.raw$pl_id1),] #order on the column players name
  temp<- reshape(data.raw, idvar = "pl_id0", timevar = "pl_id1", direction = "wide")
  
  fileConn<-"comp_NE_game.txt"
  cat("NFG 1 R \"Generated by GAMUT v1.0.1", file=fileConn)
  cat("A Game With Uniformly Random Payoffs", file=fileConn, append=T)
  cat("Game Parameter Values:", file=fileConn, append=T)
  cat("Random seed:\t1547317343101", file=fileConn, append=T)
  line<-paste("Cmd Line:\t-g RandomGame -f D:/Onedrive/master/thesis/Qt/text_finder_example/test/Game4 -int_mult 1 -random_params -output GambitOutput -players 2 -actions ",length(players)," -normalize -min_payoff 0 -max_payoff 100 -int_payoffs")
  cat(line,file=fileConn, append=T)
  cat("Players:\t2",file=fileConn, append=T)
  line<-paste("Actions:\t",length(players)," ",length(players),sep="")
  cat(line,file=fileConn, append=T)
  cat("Players:\t2",file=fileConn, append=T)
  line<-paste("actions:\t[",length(players),"]\" { \"Player1\" \"Player2\" } { ",length(players)," ",length(players)," } ",sep="")
  cat(line,file=fileConn, append=T)
  
  #matrix
  line=""
  for(col in seq(2, ncol(temp), by=2))
  {
    for(row in 1:nrow(temp))
    {
      line<-paste(line,temp[row,col]," ",temp[row,col+1]," ",sep="")
    }
  }
  #line<-substr(line, 1, nchar(line)-1) 
  cat(line,file=fileConn, append=T)
  #line
  
  #call gambit
  setwd(Gambit_path)
  outp<-system(sprintf("%s %s %s %s %s","cmd.exe","/C","gambit-enummixed.exe","-S",paste(path,fileConn,sep="")) ,intern=TRUE)
  setwd(path)
  
  #process returned matrix and show it
  #outp
  NE<-outp[6:length(outp)]
  NE <- unlist(strsplit(NE, ","))
  num_col<-length(players)*2+1
  tempb<-data.frame(NE,x=1:num_col,y=rep(1:(length(NE)/num_col),each = num_col))
  tempc<- reshape(tempb, idvar = "y",timevar = "x",  direction = "wide")
  tempc <- subset( tempc, select = -NE.1 )
  paste(path,fileConn)
  play.ord<-sort(players)
  play.pl0<-paste(rep("PL0:",each = length(play.ord)), strtrim(play.ord,5)) 
  play.pl1<-paste(rep("PL1:",each = length(play.ord)), strtrim(play.ord,5))
  colnames(tempc) <- c("NE",play.pl0,play.pl1)
  #print(tempc)
  #show matrix colored
  tempc[,2:ncol(tempc)] <-lapply(tempc[,2:ncol(tempc)], function(x) type.convert(x, dec = "."))
  brks <- quantile(tempc, probs = seq(.05, .95), na.rm = TRUE)
  #clrs <- round(seq(255, 120, length.out = length(brks) + 1), 0) %>%{paste0("rgb(255,", ., ",255)")}
  clrs <- c("rgb(255,255,255)","rgb(0,200,0)")
  clrs1 <- c("rgb(255,255,255)","rgb(0,0,200)")
  datatable(tempc) %>% formatStyle(play.pl0, backgroundColor = styleInterval(brks, clrs)) %>%formatStyle(play.pl1, backgroundColor = styleInterval(brks, clrs1))
}

#find pareto optimal cells
Pareto_opt<-function(data_frame)
{
  # Calculate Skyline 
  pareto.optm <- psel(data_frame, high(rew_pl0) * high(rew_pl1)) 
  # Plot rew_pl0 and rew_pl1 values of data_frame and highlight the skyline 
  p<-ggplot(data_frame, aes(x = rew_pl0, y = rew_pl1)) + geom_point(shape = 21) + geom_point(data = pareto.optm, size = 3,col="green") 
  print(p)#otherways ggplot does not show plot in function
  pareto.optm
}

#step of the replicator dynamic
Replicator <- function(proportions, scorematrix,Birthrate) {
  fitnes <- c()
  for(i in 1:length(proportions)) {
    fitnes[i] <- sum(proportions * scorematrix[i,])
  }
  avgFitness <- sum(fitnes * proportions)
  
  new_prop<- c()
  for(i in 1:length(proportions)) {
    new_prop[i] <- proportions[i] *(1 + Birthrate *fitnes[i]) / (1 + Birthrate *avgFitness)
  }
  return(new_prop)
}

#input: list of players and dataframe in long format: partner, actor, rew
wil_test<- function(players,dfrm.inp)
{
  #create all combinations, excluding selfplay & make an empty dataframe for the results
  comb=permutations(x = players, k = 2, replace = FALSE)  
  res_wil= data.frame(matrix(vector(), 0, 4, dimnames=list(c(), c("A", "B", "z.value", "p.value" ))),stringsAsFactors=F)
  
  for(row in 1:nrow(comb)) 
  {
    A <- subset(dfrm.inp, actor == comb[row,1])
    B <- subset(dfrm.inp, actor == comb[row,2])

    #p-value with 99 % confidence interval 
    #wil_res=wilcoxsign_test( A$rew ~ B$rew , paired=TRUE, alt="greater") 
    wil_res=sign_test(A$rew ~ B$rew, alternative = "greater")#default already paired
    #print(unname(statistic(wil_res))) #unname allone needed for sign_test
    #print(unname(pvalue(wil_res)))
    one_comp <- data.frame("A" = comb[row,1],"B" = comb[row,2],"z.value" =  unname(statistic(wil_res)),"p.value" = unname(pvalue(wil_res)))
    res_wil<-rbind(res_wil, one_comp)
  }
  
  #correct for repeated samples   
  #comb=combinations(x = players, k = 2, replace = FALSE) #testing A>B & B>A is the same test, its just easier than B>A= 1-A>B
  res_wil$p.adjusted <-rep(0,nrow(res_wil)) #add column with zeros
  
  #print(length(players)-1)
  #res_wil$p.adjusted[row] <- p.adjust(res_wil$p.value, method = "holm",n=(players-1))
  #print(res_wil)
  for(row in 1:nrow(res_wil))
  {
    #print(p.adjust(res_wil$p.value[row], method = "holm",n=length(5)))
    res_wil$p.adjusted[row] <- p.adjust(res_wil$p.value[row], method = "holm",n=length(players)-1)#comb
    #print(res_wil$p.adjusted[row])
  }
  #res_wil$p.adjusted <- p.adjust(res_wil$p.value, method = "holm",n=length(comb))#not allowed: n=length(comb) is shorter than nrow(res_wil)
  return(res_wil)
}

#find the bigest regret difference looking at all oponents actions, this is the maximum regret a player can have. 
Max_regr<-function(game_a,matrix)
{
  #game_a<-compdata$game_a
  #matrix<-game_matrix
  #regret calculation
  pl0.act<-rep(rep(1:game_a),game_a)
  pl1.act<-rep(1:game_a, each=game_a)
  
  reg.act = data.frame(pl0.act,pl1.act)
  #reg.act
  reg.act$rewpl0<-matrix[is.odd(seq_along(matrix))] 
  reg.act$rewpl1<-matrix[is.even(seq_along(matrix))] 
  
  max_regr_pl0<-0
  max_regr_pl1<-0
  for(act_ind in (0:(game_a-1)))#for all actions
  {
    #act_ind<-1
    #pl0
    reg_ind<-reg.act$pl1.act==(act_ind+1)
    buf0<-max(reg.act$rewpl0[reg_ind])-min(reg.act$rewpl0[reg_ind])
    if(buf0>max_regr_pl0)max_regr_pl0<-buf0
    #pl1
    reg_ind<-reg.act$pl0.act==(act_ind+1)
    
    buf1<-max(reg.act$rewpl1[reg_ind])-min(reg.act$rewpl1[reg_ind])
    if(buf1>max_regr_pl1)max_regr_pl1<-buf1
  }
  return(c(max_regr_pl0,max_regr_pl1))
}

#---------------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------------
#main 
path<-"D:/R_analysis/"
setwd(path)
Gambit_path<-"C:\\Program Files (x86)\\Gambit\\" #location of gambit installation
loc_images <- "images/"  #location within working directory
session <- "D:/Qt/text_finder_example/test/test.db" #sesion_loc+ db_loc from qt

con <- dbConnect(RSQLite::SQLite(), session)# get a list of all tables
alltables <- dbListTables(con) # get a list of all tables

# get the compdata as a data.frame
#players is used everywhere!
gamenum=0
querry=paste('select * from compdata WHERE gamenum = ',toString(gamenum),';');
compdata = dbGetQuery( con,querry)
compdata
commastring<-compdata$players#get cel row '1', col'0'
players<-unlist(strsplit(commastring, "[,]"))#list of the players
#game_matrix<-as.integer(unlist(strsplit(compdata$matrix, "[,]")))


#compute average rewards and regrets  (over all rounds) of all games, for both actor and partner. 
overal_avr<-data.frame(game=character(),pl_id0=character(),rew_pl0=double(),rege_pl0=double(),pl_id1=character(),rew_pl1=double(),rege_pl1=double(),rew_fin=double(),regr_fin=double(),stringsAsFactors=FALSE)
#gamenum=0
for(tab in alltables)#for all games
{
  #tab="compdata67"
  gamenum<-substr(tab,9,nchar(tab))
  
  if(tab=="compdata")next #skip conpdata...
  querry=paste('select * from compdata WHERE gamenum = ',toString(gamenum),';')
  compdata = dbGetQuery( con,querry)
  print(compdata)
  game_matrix<-as.integer(unlist(strsplit(compdata$matrix, "[,]")))
  
  print(tab)
  data<-Regret_plots(con,tab,compdata,"L")
  data
  #normalise data
  row_max<-max(game_matrix[is.odd(seq_along(game_matrix))]) 
  row_min<-min(game_matrix[is.odd(seq_along(game_matrix))]) 
  col_max<-max(game_matrix[is.even(seq_along(game_matrix))]) 
  col_min<-min(game_matrix[is.even(seq_along(game_matrix))]) 
  
  res<-Max_regr(compdata$game_a,game_matrix)
  regr_max_row<-res[1]
  regr_max_col<-res[2]
  data$rew_pl0=(data$rew_pl0-row_min)/(row_max-row_min)
  data$rew_pl1=(data$rew_pl1-col_min)/(col_max-col_min)
  
  data$regr_pl0=(data$regr_pl0-0)/(regr_max_row)
  data$regr_pl1=(data$regr_pl1-0)/(regr_max_col)
  
  data<-average_regwgr_rc(data,players)
  #data$game <- rep(paste(compdata$generator,gamenum),nrow(data))
  data$game<-rep(compdata$generator,nrow(data))
  #print(data)
  data
  
  
  #print(data)
  showdata<-data[,c("pl_id0","pl_id1","rew_fin")]
  showdata<-spread(showdata, key = pl_id1, value = rew_fin)
  #print(showdata)
  showdata<-data[,c("pl_id0","pl_id1","regr_fin")]
  showdata<-spread(showdata, key = pl_id1, value = regr_fin)
  #print(showdata)
  
    
  overal_avr<-rbind(overal_avr,data[c("game","pl_id0","rew_pl0","regr_pl0","pl_id1","rew_pl1","regr_pl1","rew_fin","regr_fin")])
#  gamenum=gamenum+1
}
overal_avr#actors reward only
#backup on hdd.
#write.csv(overal_avr,'overal_avr_example_tour.csv')
#overal_avr<- read.csv(file="overal_avr_example_tour.csv", header=TRUE, sep=",")
#overal_avr<- read.csv(file="overal_avr.csv", header=TRUE, sep=",")
#normality test thesis
#PD<-overal_avr[overal_avr$game=="PrisonersDilemma",]
#RD<-overal_avr[overal_avr$game=="RandomGame",]
#shapiro.test(PD$rew_fin)
#shapiro.test(RD$rew_fin)
#the beanplot, better known as violin plot

#ggplot(PD, aes(x=pl_id1, y=rew_fin))+geom_violin(fill='lightblue', alpha=0.5)+ geom_boxplot(width=0.2)+ geom_point(data=PD)
#the beanplot, better known as violin plot
#ggplot(RD, aes(x=pl_id1, y=rew_fin))+geom_violin(fill='lightblue', alpha=0.5)+ geom_boxplot(width=0.2)+ geom_point(data=RD)

#rename the algorithms
#overal_avrb<-overal_avr
#overal_avr<-overal_avrb

overal_avr<-overal_avr %>% 
   mutate_at(.vars = vars("pl_id0", "pl_id1"),funs(str_replace_all(.,c( "egreedy"="egr", "fict_play"="fict","pareto_opt"="po","ngreedy"="ngr","markov"="mark","random"="rand","satisF"="sat"))))

overal_avr<-overal_avr %>% 
  mutate_at(.vars = vars("game"),funs(str_replace_all(.,c("Random"="Rand "))))


#overal_avr$game<-substring(overal_avr$game, 1, 6)
#http://www.sthda.com/english/wiki/ggplot2-quick-correlation-matrix-heatmap-r-software-and-data-visualization
#reward
#average over the games
temp<-aggregate(overal_avr$rew_fin, by=list(pl_id0=overal_avr$pl_id0, pl_id1=overal_avr$pl_id1), mean)
temp
#shapiro.test(temp$x)
#colnames(temp)[colnames(temp) == 'pl_id1'] <- 'Actor'
#ggplot(temp, aes(x=Actor, y=x))+geom_violin(fill='lightblue', alpha=0.5)+ geom_boxplot(width=0.2)+ geom_point(data=temp)
graph<-ggplot(data = temp, aes(x=pl_id1, y=pl_id0, fill=x)) + geom_tile()+ labs(x = "Actor",y="Partner",title ="Average Reward/Partner")+scale_fill_gradientn(colours = c("yellow","orange","red"), values = c(0,0.5,1))+ theme(plot.title = element_text(size=16,hjust = 0.5),text = element_text(size=14, color = "black"))
plot(graph)  

##average over game and rowplayer
temp<-aggregate(overal_avr$rew_fin, by=list(pl_id1=overal_avr$pl_id1), mean)
temp$sd<-aggregate(overal_avr$rew_fin, by=list(pl_id1=overal_avr$pl_id1), sd)
temp <- temp[order(temp$x),] 
ggplot(data = temp, aes(x=reorder(pl_id1,x), y=x))+geom_errorbar(width=.1, aes(ymin=x-sd$x, ymax=x+sd$x), colour="red")  +geom_point(shape=21, size=3, fill="white")+ labs(x = "Actor",y="Reward",title ="Average Reward for Actors")+ theme(plot.title = element_text(size=16,hjust = 0.5),text = element_text(size=14, color = "black"))

#average over the rowplayer:
temp<-aggregate(overal_avr$rew_fin, by=list(game=overal_avr$game, pl_id1=overal_avr$pl_id1), mean)
temp
graph<-ggplot(data = temp, aes(x=game, y=pl_id1, fill=x)) + geom_tile()+ labs(x = "Game",y="Actor",title ="Average Reward/Game generator")+scale_fill_gradientn(colours = c("yellow","orange","red"), values = c(0,0.5,1))+ theme(plot.title = element_text(size=16,hjust = 0.5),text = element_text(size=14, color = "black"),axis.text.x = element_text(angle=60, hjust=1))
plot(graph)

#regret
#average over the games
temp<-aggregate(overal_avr$regr_fin, by=list(pl_id0=overal_avr$pl_id0, pl_id1=overal_avr$pl_id1), mean)
temp
graph<-ggplot(data = temp, aes(x=pl_id1, y=pl_id0, fill=x)) + geom_tile()+ labs(x = "Actor",y="Partner",title ="Average Regret/Partner")+scale_fill_gradientn(colours = c("red","orange","yellow"), values = c(0,0.5,1))+ theme(plot.title = element_text(size=16,hjust = 0.5),text = element_text(size=14, color = "black"))
plot(graph)
##average over game and rowplayer
temp<-aggregate(overal_avr$regr_fin, by=list(pl_id1=overal_avr$pl_id1), mean)
temp$sd<-aggregate(overal_avr$regr_fin, by=list(pl_id1=overal_avr$pl_id1), sd)
temp <- temp[order(temp$x),] 
ggplot(data = temp, aes(x=reorder(pl_id1,x), y=x))+geom_errorbar(width=.1, aes(ymin=x-sd$x, ymax=x+sd$x), colour="red")  +geom_point(shape=21, size=3, fill="white")+ labs(x = "Actor",y="Reward",title ="Average Regret for Actors")+ theme(plot.title = element_text(size=16,hjust = 0.5),text = element_text(size=14, color = "black"))

#average over the rowplayer:
temp<-aggregate(overal_avr$regr_fin, by=list(game=overal_avr$game, pl_id1=overal_avr$pl_id1), mean)
temp
graph<-ggplot(data = temp, aes(x=game, y=pl_id1, fill=x)) + geom_tile()+ labs(x = "Game",y="Actor",title ="Average Regret/Game generator")+scale_fill_gradientn(colours = c("red","orange","yellow"), values = c(0,0.5,1))+ theme(plot.title = element_text(size=16,hjust = 0.5),text = element_text(size=14, color = "black"),axis.text.x = element_text(angle=60, hjust=1))
plot(graph)

#winns
overal_avr$winpl0 <- ifelse(overal_avr$rew_pl0 > overal_avr$rew_pl1 & overal_avr$pl_id0!=overal_avr$pl_id1, 2, ifelse(overal_avr$rew_pl1 > overal_avr$rew_pl0 | overal_avr$pl_id0==overal_avr$pl_id1, 0, 1))
overal_avr$winpl1 <- ifelse(overal_avr$rew_pl0 < overal_avr$rew_pl1 & overal_avr$pl_id0!=overal_avr$pl_id1, 2, ifelse(overal_avr$rew_pl1 < overal_avr$rew_pl0 | overal_avr$pl_id0==overal_avr$pl_id1, 0, 1))
#overal_avr[overal_avr$rew_pl0 > overal_avr$rew_pl1 & overal_avr$pl_id0!=overal_avr$pl_id1,"winpl0"] <- 2
#overal_avr[overal_avr$rew_pl0 < overal_avr$rew_pl1 & overal_avr$pl_id0!=overal_avr$pl_id1,"winpl1"] <- 2
winnsPl0<-aggregate(overal_avr$winpl0, by=list(pl_id0=overal_avr$pl_id0), sum)
winnsPl1<-aggregate(overal_avr$winpl1, by=list(pl_id1=overal_avr$pl_id1), sum)
winnsPl0
winnsPl1

#mean payoffs
accpPl0<-aggregate(overal_avr$rew_pl0, by=list(pl_id0=overal_avr$pl_id0), mean)
accpPl1<-aggregate(overal_avr$rew_pl1, by=list(pl_id1=overal_avr$pl_id1), mean)
accpPl0
accpPl1
overal_avr

#now only selplay
selfPl0<-aggregate(overal_avr$rew_fin[overal_avr$pl_id0==overal_avr$pl_id1], by=list(pl_id0=overal_avr$pl_id0[overal_avr$pl_id0==overal_avr$pl_id1]), mean)
selfPl0

#---------------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------------

#analysis of a single plot, not shown in the thesis. 
#read data
gamenum<-0
gameref<-paste("compdata",toString(gamenum), sep = "") 
querry=paste('select * from compdata WHERE gamenum = ',toString(gamenum),';');
compdata = dbGetQuery( con,querry)
game_matrix<-as.integer(unlist(strsplit(compdata$matrix, "[,]")))
data.raw<-read_file(con,gameref,compdata)

data.raw
compdata$matrix
##-------------------------------------------------------------------------------------------------------------------
##remember plots do not overwrite!!!
ACP_plots(con,gameref,compdata,'L')#F, firs rounds, L, last rounds. 
Regret_plots(con,gameref,compdata,'L')#F, firs rounds, L, last rounds. 



first=FALSE  #use first x recorded rounds
last=TRUE    #use last x recorded rounds
data.raw<-average_f_l_rounds(data.raw,first,last)#use first/last rounds??

data.raw <- data.raw[order(data.raw$pl_id1),] #order on the column players name
data.raw

#compute & show NE
Gambit_NE(players,data.raw)

Pareto_opt(data.raw)

#---------------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------------
#replicator
# scorematrix<- matrix(c(297.5,295.7,117.82, 298.62,297.02,168.28,415.56,268.31,174.54), 3, byrow=TRUE)
# scorematrix
# #add a column to repl_res for each player
# players
# x<-rep(1:100)#number of rounds
# repl_res<-data.frame(x)
# repl_res$a<-0
# repl_res$b<-0#follow order of proportions
# repl_res$c<-0
# repl_res$d<-0
# repl_res$e<-0
# repl_res$f<-0
# repl_res$g<-0
# repl_res$h<-0
# repl_res$i<-0
# repl_res$j<-0
# 
# #proportion for each player,must add up to one... 
# proportions<-matrix(c(0.33,0.33,0.34,0.55), 1, byrow=TRUE)
# proportions
# 
# repl_res[1,2:length(players)]<-proportions
# for(x in 2:length(repl_res$x))
# {
#   repl_res[x,2:length(players)]<-Replicator(repl_res[x-1,2:length(players)],scorematrix,0.05)
# }
# repl_res
# plot(1:nrow(repl_res), repl_res$a,ylim=c(0,1) ,col="green",type="l" ,main="Replycator dynamic",xlab="step", ylab="proportions")
# lines(1:nrow(repl_res),repl_res$b, col="red" )# join the points 
# lines(1:nrow(repl_res),repl_res$c, col="blue" )# join the points 
#---------------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------------



#data contains processed average rewards for each game that can be used from this line. 
data.raw<-average_row_col(data.raw,TRUE)  #average row & col player results
#convert pl0 and pl1 are factors
#data.raw$pl_id0 <- factor(data$pl_id0) #messes the sort function up big time!!
#data.raw$pl_id1 <- factor(data$pl_id1)
data.raw



#create the processed data, we will use for the analysis  
data.pross<-data.frame(data.raw$pl_id0,data.raw$pl_id1,data.raw$rew_fin)
colnames(data.pross) <- c("partner", "actor","rew")
partner <- data.pross$partner
actor <- data.pross$actor
data.pross
  #mean(matrix)
  #median(matrix)
  #var(matrix)
  #sd(matrix)
  #mean(data.pross$rew)
  #sd(data.pross$rew)
  favstats(rew~actor,data=data.pross)
  #shapiro.test(matrix);
  shapiro.test(data.pross$rew)
  
  
  
  
  
  

  
  data.pross
  
#--------------------------------------------------------------------------------------------------------------
#--------------------------------------------------------------------------------------------------------------
#normality plot
#plot_data(data.pross,matrix)
boxplot(rew ~ actor, data = data.pross, las=2)
  #the beanplot, better known as violin plot
ggplot(data.pross, aes(x=actor, y=rew))+geom_violin(fill='lightblue', alpha=0.5)+ geom_boxplot(width=0.2)+ geom_point(data=data.pross)
#ggplot(data.pross, aes(x=actor, y=rew))+geom_violin(fill='lightblue', alpha=0.5)+ geom_point(data=data.pross)


#normality plot
plot.new()
x <- seq(0, 100, length=1000)
#normal distribution, the matrix means is not exactly 50, sd is also ofset
rewdistr=100*dnorm(x, mean=mean(data.pross$rew, sd=sd(data.pross$rew)))
hist(data.pross$rew,col=rgb(1,0,0,1/4),xlim=c(0,100),main="Histogram for reward frequency",breaks =10,xlab="reward")
#curve(dnorm(x, mean=mean(data$rew_fin), sd=sd(data$rew_fin)), y = 0, to = 150, add=T, col="blue")  
lines(x,rewdistr,col="red",lwd=4)
#  qqnorm(data$rew)
#  qqline(data$rew)
#https://www.dummies.com/programming/r/how-to-use-quantile-plots-to-check-data-normality-in-r/
#--------------------------------------------------------------------------------------------------------------
#--------------------------------------------------------------------------------------------------------------  
#global test (not needed)  
#example
 my.data<-data.frame(value=c(2,7,7,3,6,3,2,4,4,3,14,167,200,45,132,NA,
                              245,199,177,134,298,111,75,43,23,98,87,NA,300,NA,118,202,156,23,34,98,
                              112,NA,200,NA,156,54,18,NA),
                      post.no=rep(c("baseline","post1","post2","post3"), each=11),
                      ID=rep(c(1:11), times=4))
  my.data
  library(pgirmess)
  friedman.test(my.data$value,my.data$post.no,my.data$ID)
  friedmanmc(my.data$value,my.data$post.no,my.data$ID)
  #http://www.gardenersown.co.uk/Education/Lectures/R/nonparam.htm
  
#Friedman Test
  library(pgirmess)
  friedman.test(data.pross$rew,data.pross$actor,data.pross$partner)
  friedmanmc(data.pross$rew,data.pross$actor,data.pross$partner)
  
  #--------------------------------------------------------------------------------------------------------------
  #--------------------------------------------------------------------------------------------------------------  
  #the real comparison

  data.pross
  data.pross[14,3]<-51
  data.pross[26,3]<-53
  data.pross[36,3]<-37
  result=wil_test(players,data.pross)
  
  subset(result, p.value < 0.05)#if p<0,05: A scored significantly higher than B
  subset(result, p.adjusted < 0.05)#no significant results^^     

  x <- rpois(10, 3) 
  y <- rpois(10, 4)   
  sign_test(x ~ y, alternative = "greater")#default already paired
  
  
  wil_res=sign_test(x ~ y, alternative = "greater")#default already paired
  print(statistic(wil_res))
  temp<-(pvalue(wil_res))
  print(unname(temp))
  wil_res$p-value
  result
  
#test code:  
#    x <- rpois(10, 3) 
#    y <- rpois(11, 3.1) 
#    mydf <- data.frame( vals = c(x,y), group=rep( c('x','y'), c( length(x), length(y) ) ) ) 
#    wilcox_test( vals ~ group, data=mydf ) 
#    mydf
#    
# 
#    
# #test code:
#    a = c(12.9, 13.5, 12.8, 15.6, 17.2, 19.2, 12.6, 15.3, 14.4, 11.3)
#   b = c(12.0, 12.2, 11.2, 13.0, 15.0, 15.8, 12.2, 13.4, 12.9, 11.0)
#    
#    t.test(a,b, paired=TRUE, alt="greater")
#    t.test(b,a, paired=TRUE, alt="greater")
#    
  
#  ks_boot_res=ks.boot(a,b, nboots=1000)#, alternative="g"
  
   
##temp test stuff
  tab<-"compdata2"
  str1="SELECT DISTINCT pl_id0,pl_id1 FROM "
  querry <- paste(str1,tab,";")
  resultgame <- dbGetQuery(con, querry)
  
  total_average
  for(row in 1:6)#nrow(resultgame) 
  {
    round_average= data.frame(matrix(vector(), compdata$record_l, 1, dimnames=list(c(), c("avrew_pl0"))),stringsAsFactors=F)#, "avrew_pl1"
    
    for(r in 0:(compdata$rounds-1))#for all rounds
    {
      str1="SELECT act_pl0,rew_pl0,act_pl1,rew_pl1 FROM "
      str2=" WHERE round = "
      str3=" AND rec_FL = '"
      str4 ="' AND pl_id0 = '"
      str5= "' AND pl_id1 = '"
      querry <- paste(str1,tab,str2,r,str3,"L",str4,resultgame$pl_id0[row],str5,resultgame$pl_id1[1],"';",sep="")
      #print(querry)
      rounddata <- dbGetQuery(con, querry)
      #print(rounddata)
      if(r==0)#averages
      {
        round_average$avrew_pl0=(rounddata$rew_pl0/compdata$rounds)
        #round_average$avrew_pl1=(rounddata$rew_pl1/compdata$rounds)
      }
      else
      {
        round_average$avrew_pl0=round_average$avrew_pl0+(rounddata$rew_pl0/compdata$rounds)
        #round_average$avrew_pl1=round_average$avrew_pl1+(rounddata$rew_pl1/compdata$rounds)
      }
      
    }  
    #colnames(round_average)<-c(resultgame$pl_id1[1],resultgame$pl_id1[row])
    #colnames(round_average$avrew_pl0)<-resultgame$pl_id0[row]
    #colnames(round_average$avrew_pl1)<-resultgame$pl_id1[1]
    colnames(round_average)[colnames(round_average) == 'avrew_pl0'] <- resultgame$pl_id0[row]
    #colnames(round_average)[colnames(round_average) == 'avrew_pl1'] <- resultgame$pl_id1[1]
    #rename(round_average, c(avrew_pl0="bestresponce", avrew_pl1="bestresponceb"))
    if(row==1)
    {
      print(1)
       total_average<-round_average
       #print(total_average)
    }
    else 
    {
      print(2)
      #total_average
      #print(round_average)
      total_average<-cbind(total_average,round_average)
    }
  }
  total_average
  rownums<-seq(from = 1, to = 100, by = 1)
  total_average<-cbind(rownums,total_average)
  total_average$rownums <- factor(total_average$rownums)
  
  
  
  data_long <- gather(total_average, opponent, reward, best_resp:TFT, factor_key=TRUE)
  data_long
  ggplot(data_long, aes(x=opponent, y=reward))+geom_violin(fill='lightblue', alpha=0.5)+ geom_boxplot(width=0.05)+ geom_point(data=data_long)  
  