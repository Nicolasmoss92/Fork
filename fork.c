#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <time.h> //clock(), CLOCKS_PER_SEC e clock_t

#define KEY 1   // chave

//******************VARIAVEIS GLOBAIS*************************
   pid_t pid;
   clock_t t;
   char linhaAtual[95], maiorAno[6];
   char *token;
   const char s[2] = ";";
   int shmidAutores, shmid,c, k=0, j=0,contadorLivrosTotal=0, contadorAnosTotal=0, maiorNumeroEdicoes=0, novoItem;
   key_t key = KEY;
   FILE *pFile, *pWrite;  
  
   struct livro{
   	char codigo [8];
   	char titulo [47];
   	char autor[32];
   	char edicao[3];
   	char ano[6];
   };
   struct autores{
   	char nome[34];
   	int quantidade;
   };
   struct anos{
   	char year [6];
   };
   struct publicados{
        char tituloPublicado [47];
        struct anos vetorAnos[100]; 
        int contadorAnos;
   };
   struct edicao{
   	char ano[6];
   	int numeroEdicoes;
   };
  
//****************** FIM VARIAVEIS GLOBAIS*************************

//LER ARQUIVO E SEPARAR AUTORES******************************************************************************************
    struct livro* separaLivros(){
    struct livro *vetorLivros = malloc (2000000* sizeof (struct livro)); //alocar memória para um vetor de livros
   //ARQUIVO
    FILE *pFile;
    pFile = fopen ("livros2.txt", "r" );

   //SEPARAR LIVROS POR ;

   while (k< 2000000){
         fgets(linhaAtual,95, pFile);   	
	strcpy(linhaAtual,strtok(linhaAtual, "\n"));
   	token = strtok(linhaAtual, s); 
   
		//colocar o código do livro no vetor	
   		strcpy(vetorLivros[k].codigo, token );
   		//printf( "CODIGO: %s\n", vetorLivros[k].codigo );
		token = strtok (NULL, s);
	   
		//colocar o título do livro
		strcpy(vetorLivros[k].titulo, token );
          	//printf("TITULO:  %s\n", vetorLivros[k].titulo );
		token = strtok(NULL, s);
	
		//colocar autor
		strcpy(vetorLivros[k].autor, token );
		//printf( "AUTOR = %s\n", vetorLivros[k].autor );
		token = strtok (NULL, s);

	
		//colocar edição
		strcpy(vetorLivros[k].edicao, token );
		//printf( "EDICAO: %s\n", vetorLivros[k].edicao );
		token = strtok (NULL, s);
	
		//colocar ano
		strcpy(vetorLivros[k].ano, token );
	//	printf( "ANO: %s\n\n\n", vetorLivros[k].ano );
		token = strtok (NULL, s);		 
   k=k+1;
   j++;	
   }
   
   return vetorLivros;
}
//****************************************************************************************************************************
//----------------------------------------------------------------------------------------------------------------------------   


//****************************************************************************************************************************
struct autores* separaAutores(struct livro *vetorLivrosRecebido, struct autores *vetorAutoresSHM, int indiceEntrada, int indiceSaida){
   struct livro *vetorLivros = vetorLivrosRecebido;
   struct autores *vetorAutores = vetorAutoresSHM;
      
   //--------------------------------------------- SEPARAR OS AUTORES PELO NOME ---------------------------------------------
      for (k=indiceEntrada; k<indiceSaida; k++){
     vetorAutores[k].quantidade = 0;
   }
       
            novoItem = indiceEntrada;
	   for (k=indiceEntrada; k<indiceSaida;k++){
	   
	   	for (j = indiceEntrada; j<k; j++){
	   	
	     		if ( strcmp(vetorAutores[j].nome, vetorLivros[k].autor) == 0 ) {
	     		vetorAutores[j].quantidade= vetorAutores[j].quantidade +1;
	     		j = k+1;
	     		}    
	   	}
	   	
	     if (k==j){ //se K == J então ele não achou nenhum igual e insere no vetor de autores
	     	strcpy(vetorAutores[novoItem].nome, vetorLivros[k].autor);
	     	vetorAutores[novoItem].quantidade = vetorAutores[novoItem].quantidade+1 ;
	     	novoItem++;
	     }
	   }
	   return vetorAutores;
}
//****************************************************************************************************************************
//----------------------------------------------------------------------------------------------------------------------------        
         
         
//--------------------------------------------- SEPARAR OS LIVROS PELO NOME E ANO---------------------------------------------
void filtraLivros(struct livro *vetorLivrosRecebido){
     struct livro *vetorLivros = vetorLivrosRecebido;
     struct  publicados *livrosPublicados;
     shmid = shmget(key, 2000000 * sizeof(struct livro), IPC_CREAT | 0600 );
     livrosPublicados = shmat(shmid, 0, 0); 
       pWrite = fopen ("livrosPublicados.txt", "w"); // cria um arquivo para os livros publicados
        int flag = 0;
       for (k=0; k<100000; k++){
         livrosPublicados[k].contadorAnos = 0;
       }
       
           novoItem = 0;
	   for (k=0; k<100000;k++){
	   
	   	for (j = 0; j<k; j++){
	   	
	     		if ( strcmp(livrosPublicados[j].tituloPublicado, vetorLivros[k].titulo) == 0 ) {
	     		//j = k+1; //pula do for interno
	     		break;
	     		}    
	   	}
	   	
		     if (k==j){ //se K == J então ele não achou nenhum igual e insere no vetor de livros publicados
		     	strcpy(livrosPublicados[novoItem].tituloPublicado, vetorLivros[k].titulo);
		     	strcpy(livrosPublicados[novoItem].vetorAnos[0].year, vetorLivros[k].ano); 
		     	//                                      {posição do vetor de anos de dentro da struct dos livros publicados]
		     	livrosPublicados[novoItem].contadorAnos++; //incrementa o contador de anos para quando for inserir um novo ano, inserir em uma nova posição
		     	contadorLivrosTotal++;
		     	novoItem++;
		     }
		     else{ //se k for diferente de j quer dizer que achou um titulo igual, ai apenas concatena a string do ano CASO JÃ NAO HAJA O ANO ANTES
		    	 //procurar se há um ano igual
		    	 for (c=0; c < livrosPublicados[j].contadorAnos; c++) {
		    	 flag = 0;
		    	 	if (strcmp (vetorLivros[k].ano, livrosPublicados[j].vetorAnos[c].year) == 0) {
		    	 	flag++;
		    	 	}
		    	 	
		    	 }
		    		if (flag == 0){ //flag == 0 quer dizer que nenhum ano do vetor de anos é igual ao livro atual, então pode inserir o ano no vetor
		    		 strcpy(livrosPublicados[j].vetorAnos[ livrosPublicados[j].contadorAnos ].year, vetorLivros[k].ano); //concatena strings 
		    		 livrosPublicados[j].contadorAnos++; //incrementa o contador de anos para quando for inserir um novo ano, inserir em uma nova posição
		     		}
		     }
	   }
	         
   for (k = 0; k < 100000; k++){
   fprintf (pWrite, "Nome do Livro: %s\n", livrosPublicados[k].tituloPublicado);
   fprintf (pWrite, "ANOS DE PUBLICAÇÃO DESTE LIVRO:\n");
        for(c=0; c < livrosPublicados[k].contadorAnos ; c++){
        fprintf (pWrite, "%s, ", livrosPublicados[k].vetorAnos[c].year );
        }
   fprintf (pWrite, "\n---------------------------------------------------------------------------\n\n");
   }
   shmdt(livrosPublicados); //libera memória compartilhada
   shmctl(shmid, IPC_RMID, NULL);

}
//***********************************************************************************************************************************
//-----------------------------------------------------------------------------------------------------------------------------------

//***********************************************************************************************************************************

void separaEdicoes(struct livro *vetorLivrosRecebido){
    //--------------------------------------------- SEPARAR AS EDICOES POR ANO ---------------------------------------------
       struct livro *vetorLivros = vetorLivrosRecebido;
       
       pWrite = fopen ("edicoes.txt", "w"); // cria um arquivo para as edições
       struct  edicao *vetorEdicoes;
       shmid = shmget(key, 2000000 * sizeof(struct livro), IPC_CREAT | 0600 );
       vetorEdicoes = shmat(shmid, 0, 0); 
       
      for (k=0; k<100000; k++){
     vetorEdicoes[k].numeroEdicoes = 0;
   }
   	   novoItem = 0;
	   for (k=0; k<100000;k++){
	   
	   	for (j = 0; j<k; j++){
	   	
	     		if ( strcmp(vetorEdicoes[j].ano, vetorLivros[k].ano) == 0 ) {
	     		vetorEdicoes[j].numeroEdicoes= vetorEdicoes[j].numeroEdicoes + 1;
	     		j = k+1;
	     		}    
	   	}
	   	
	     if (k==j){ //se K == J então ele não achou nenhum ano igual e insere no vetor de edições
	     	strcpy(vetorEdicoes[novoItem].ano, vetorLivros[k].ano);
	     	vetorEdicoes[novoItem].numeroEdicoes = vetorEdicoes[novoItem].numeroEdicoes + 1 ;
	     	contadorAnosTotal++;
	     	novoItem++;
	     }
	   }
	         
   for (k = 0; k < 100000; k++){
   fprintf (pWrite, "ANO: %s -----> EDIÇOES: %d\n", vetorEdicoes[k].ano, vetorEdicoes[k].numeroEdicoes );
   }

   //--------------------------------------------- FIM SEPARAR AS EDICOES POR ANO ---------------------------------------------
   //ITEM 1
   printf ("Contador de livros Total: %d\n", contadorLivrosTotal); //este contador está no laço do SEPARAR OS LIVROS PELO NOME E ANO
   //ITEM 2
   for (k=0; k < 100000; k++){
	   if ( vetorEdicoes[k].numeroEdicoes > maiorNumeroEdicoes){
	   strcpy (maiorAno, vetorEdicoes[k].ano);
	   maiorNumeroEdicoes = vetorEdicoes[k].numeroEdicoes;
	   }
   }
   printf ("QTD DE ANOS: %d\n MAIOR ANO: %s\n", contadorAnosTotal, maiorAno); 
   shmdt(vetorEdicoes); //libera memória compartilhada
   shmctl(shmid, IPC_RMID, NULL);   
}
//***********************************************************************************************************************************
//-----------------------------------------------------------------------------------------------------------------------------------

void main () {

     struct livro *passarVetorLivros = separaLivros(); //separa os autores por ; e aponta para o vetor com o passarVetorLivros
     
     //criar memória compartilhada para o vetor de autores. Depois, vários processos irão alimentar essa área
      struct autores *vetorAutoresSHM;
      shmidAutores = shmget(key, 2000000 * sizeof(struct livro), IPC_CREAT | 0600 );
      vetorAutoresSHM = shmat(shmidAutores, 0, 0); 
     	if ( ( shmid = shmget (key, 2000000 * sizeof(struct livro), IPC_CREAT | 0600)) < 0 )
	printf("Criação de memória compartilhada com falha.\n");

    
     separaAutores(passarVetorLivros, vetorAutoresSHM, 0, 50000);
     pid = fork();
     if (pid > 0){
     	
     	pid = fork();
     	if (pid > 0){
       	
     		pid = fork();
     		if (pid >0){
     			 separaAutores(passarVetorLivros,vetorAutoresSHM, 50001, 100000);   
     			 wait(NULL);
     			 wait(NULL);
     			 wait(NULL);
     			      filtraLivros (passarVetorLivros);
    			      separaEdicoes(passarVetorLivros);
   	  			 	
     	 	}else separaAutores(passarVetorLivros,vetorAutoresSHM, 100001, 150000);			     		

     	}else separaAutores(passarVetorLivros,vetorAutoresSHM, 150001, 200000);

     }
     else {
     
                 pWrite = fopen ("autoresFork.txt", "w"); // cria um arquivo para os autores. Ultimo processo gera os livros 
		for (k = 0; k < 200000; k++){
		    fprintf (pWrite, "AUTOR: %s -----> Quantidade: %d\n", vetorAutoresSHM[k].nome, vetorAutoresSHM[k].quantidade );
	      	}
     }
     
 
     
     
     shmctl(shmidAutores, IPC_RMID, NULL);   

}
