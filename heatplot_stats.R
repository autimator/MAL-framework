
library("coin")#for wilcox_test
library("BSDA")#sign test, wilcox alternative
library("arrangements")#make all combinations of players
library("ggplot2")#plots
library("tidyverse")
#
#
#https://stackoverflow.com/questions/14290364/heatmap-with-values-ggplot2
#

#Reward heatplot with significance levels.


#input: list of players and dataframe in long format: partner, actor, rew
sign_tests<- function(mode,dfrm.inp)
{
  #mode="p"
  #dfrm.inp<-overal_avr
    
  games<-as.character(unique(dfrm.inp$game))
  players<-as.character(unique(dfrm.inp$pl_id0))
  #create all combinations, excluding selfplay & make an empty dataframe for the results
  
  com_pl1=permutations(x = players, k = 2, replace = FALSE)
  
  if(mode=="p")
  {
    #comb=permutations(x = players, k = 2, replace = FALSE)
    comb<-players
  } else  {
    #comb=permutations(x = games, k = 2, replace = FALSE)  
    comb<-games
  }
  sign_res= data.frame(matrix(vector(), 0, 5, dimnames=list(c(), c("grp","pl_id0", "pl_id1", "z.value", "p.value" ))),stringsAsFactors=F)
  
  
  #for(row in 1:length(comb)) #nrow(comb)
  #{
    #row<-1
    for(rowb in 1:nrow(com_pl1)) 
    {
      #row=1
      #rowb=1
      # if(mode=="p")
      # {
      #   A <- subset(dfrm.inp, pl_id1==com_pl1[rowb,1]  )#& pl_id0 == comb[row]
      #   B <- subset(dfrm.inp, pl_id1==com_pl1[rowb,2] )#& pl_id0 == comb[row]
      # } else {#mode g, game
      # 
      #   A <- subset(dfrm.inp, pl_id1==com_pl1[rowb,1] &  game == comb[row])
      #   B <- subset(dfrm.inp, pl_id1==com_pl1[rowb,2] &  game == comb[row])
      # }
      select <- subset(dfrm.inp,pl_id0==com_pl1[rowb,1] & pl_id1==com_pl1[rowb,2]  )
      
      #p-value with 99 % confidence interval 
      #wil_res=wilcoxsign_test( A$rew ~ B$rew , paired=TRUE, alt="greater") 
      #wil_res=sign_test(A$x ~ B$x, alternative = "greater")#default already paired
      
      
      one_res<-sign_test(select$rew_pl0 ~ select$rew_pl1, alternative = "less")#default already paired
      
      #print(unname(statistic(wil_res))) #unname allone needed for sign_test
      #print(unname(pvalue(wil_res)))
      one_comp <- data.frame("grp"=comb[1],"pl_id0" = com_pl1[rowb,1],"pl_id1" = com_pl1[rowb,2],"z.value" =  unname(statistic(one_res)),"p.value" = unname(pvalue(one_res)))
      sign_res<-rbind(sign_res, one_comp)
    }
  #}
  
  #correct for repeated samples   
  #testing A>B & B>A is the same test, its just easier than B>A= 1-A>B
  sign_res$p.adjusted <-rep(0,nrow(sign_res)) #add column with zeros
  

  for(row in 1:nrow(sign_res))
  {
    sign_res$p.adjusted[row] <- p.adjust(sign_res$p.value[row], method = "holm",n=length(players)-1)#comb
  }
  return(sign_res)
}


#test data
#players<-c("b","c","d")
#game   <-c("one","one","one","one","one","one","one","one","one","two","two","two","two","two","two","two","two","two","three","three","three","three","three","three","three","three","three")
#pl_id0 <-c("b","b","b","c","c","c","d","d","d","b","b","b","c","c","c","d","d","d","b","b","b","c","c","c","d","d","d")
#pl_id1 <-c("b","c",'d',"b","c",'d',"b","c",'d',"b","c",'d',"b","c",'d',"b","c",'d',"b","c",'d',"b","c",'d',"b","c",'d')
#x<-c(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27)
#data.pross<- data.frame(game,pl_id0,pl_id1,x)


#Loaded from main analysis file. 
overal_avr<- read.csv(file="overal_avr.csv", header=TRUE, sep=",")

overal_avr<-overal_avr %>% 
  mutate_at(.vars = vars("pl_id0", "pl_id1"),funs(str_replace_all(.,c( "egreedy"="egr", "fict_play"="fict","pareto_opt"="po","ngreedy"="ngr","markov"="mark","random"="rand","satisF"="sat"))))

overal_avr<-overal_avr %>% 
  mutate_at(.vars = vars("game"),funs(str_replace_all(.,c("Random"="Rand "))))


result=sign_tests("g",overal_avr)

#subset(result, p.value < 0.05)#if p<0,05: A scored significantly higher than B
low_sig<-subset(result, p.value < 0.05)   #p.adjusted
high_sig<-subset(result, p.value < 0.001)
#
#prep for the plots!!
#


#average over the games
temp<-aggregate(overal_avr$rew_fin, by=list(pl_id0=overal_avr$pl_id0, pl_id1=overal_avr$pl_id1), mean)
temp

#add significance
temp$signif<-""

for(rowb in 1:nrow(temp))
{
  for(row in 1:nrow(low_sig))
  {
    if(low_sig$pl_id0[row]==temp$pl_id0[rowb] & low_sig$pl_id1[row]==temp$pl_id1[rowb])
    {temp$signif[rowb]<-"*"}

  }

  for(row in 1:nrow(high_sig))
  {
      if(high_sig$pl_id0[row]==temp$pl_id0[rowb] & high_sig$pl_id1[row]==temp$pl_id1[rowb])
        {temp$signif[rowb]<-"**"}
  }  
}
 


#plot result                 
graph<-ggplot(data = temp, aes(x=pl_id1, y=pl_id0, fill=x)) + geom_tile()+ labs(x = "Actor",y="Partner",title ="Average Reward/Partner")+scale_fill_gradientn(colours = c("yellow","orange","red"), values = c(0,0.5,1))+ theme(plot.title = element_text(size=16,hjust = 0.5),text = element_text(size=14, color = "black")) +geom_text(size=10,aes(label = signif))
plot(graph) 

low_sig
high_sig

pr<-subset(overal_avr, (pl_id0=="rand" ) & pl_id0!= pl_id1) #1010 as actor and 1010 as partner = 2020- 1919= 101 selfplay...
count()
