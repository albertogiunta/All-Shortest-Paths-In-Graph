#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<limits.h>

#define VUOTO -1
#define NIL 0
#define INFINITO 111
#define TRUE 1
#define FALSE 0

							/* STRUTTURE DATI */
/*-------------------------------------------------------------------------------*/
typedef struct VERTEX {
    int id;
    int dist;
	int padre;
    struct VERTEX *next;
} VERTEX;

typedef struct PADRI {
	int id;
	struct PADRI *next;
    struct PADRI *prec;
} PADRI;

typedef struct PILA {
    int id;
    PADRI *vettPadri;	//testa lista padri
    struct PILA *next;
} PILA;

typedef struct GRAPH {
	// Stuttura dati grafo
    VERTEX **AdjList ;	// Lista di adiacenza
    int n;				// Numero di vertici nel grafo
    PADRI **p;			// Array di liste dei padri di ogni nodo
    int *inPila;		// flag In pila (se il nodo è attualmente in pila oppure no
	int *d;				// Distanza di ogni nodo dalla sorgente
} GRAPH;

							/* DEFINIZIONE FUNZIONI */
/*-------------------------------------------------------------------------------*/

VERTEX *VertexAlloc (int id , int dist);
VERTEX **AdjListRead (char * file , int * n) ;
GRAPH  *GraphAlloc (int n) ;
GRAPH  *GraphRead (char * file);

void Initialize( GRAPH *G ,int s, int* V);
void eliminaCorrispondenze (PADRI **list);
void Relax (int u,int v,GRAPH *G);
int	 EmptyQueue (GRAPH* G,int V[]);
int  Svuota (GRAPH* G,int V[], int s);
int  ExtractMin (GRAPH* G,int *V);
void Dijkstra (GRAPH *G, int s, int *V);

void   push (PILA** testaPila, int nodo, PADRI* listaPadri);
PADRI* pop (PILA** testaPila, int nodo);

void stampaCamminiMinimi (GRAPH *G, int nodo, int sorg, int ultimoNodo, PILA **pila, int *flagScorrimento);
void stampaDoppi (GRAPH *G, int nodo, int sorg);
void DeleteGraph(GRAPH* G);
void PrintAllSP(GRAPH *G);


						/* MAIN + PRINTALLSP + DELETEGRAPH */
/*-------------------------------------------------------------------------------*/

int main(int argc, char *argv[]) {
	GRAPH *G;
    
    	if(argc!=2) {
    		fprintf(stderr,"Usage: allsp <file>\n");
    		return 1;
    	}
    if ((G=GraphRead(argv[1]))==NULL) {
        fprintf(stderr,"Error reading %s\n",argv[1]);
        return 1;
    }
	PrintAllSP(G);
	DeleteGraph(G);
    
	return 0;
}

/////////////////////////////////////

void PrintAllSP(GRAPH *G) {
	// Procedura di stampa di tutti gli shortest paths nel grafo G
	int scorriSorgenti, nodo, flagScorrimento;
//	int sorg;
	int *V = (int*)malloc(sizeof(int)*G->n);
	if(V == NULL) {
		printf("Non c'e' abbastanza memoria\n");
		system("pause");
		exit(1);
	}
	
    PILA *testaPila = (PILA*)malloc(sizeof(PILA));
    if(testaPila== NULL) {
		printf("Non c'e' abbastanza memoria\n");
		system("pause");
		exit(1);
	}
    testaPila->next = NULL;
	
    //scorri tutte le sorgenti
	for (scorriSorgenti = 0; scorriSorgenti < G->n; scorriSorgenti++) {
				
//        sorg = scorriSorgenti;
		Dijkstra(G, scorriSorgenti, V);
        
        //scorre tutti i nodi e stampa i cammini dalla sorgente corrente (indicata da scorriSorgenti)
		for (nodo = 0; nodo < G->n; nodo++) {
			if (nodo == scorriSorgenti) nodo++;
			if (nodo == G->n) break;
			do {
                flagScorrimento = FALSE;
				stampaCamminiMinimi (G, nodo, scorriSorgenti, nodo, &testaPila, &flagScorrimento);
				printf("\n");
			} while (flagScorrimento != 0);
		}
	}
}

/////////////////////////////////////

void DeleteGraph(GRAPH *G) {
	// Procedura per la distruzione di un grafo
	int i;
	
	//cancello la lista dei padri
	PADRI* canc;
	for(i = 0; i < G->n; i++) {
		while(G->p[i] != NULL) {
			canc = G->p[i];								//Assegno a canc (puntatore d'appoggio)il valore dell' elemento
			G->p[i] = G->p[i]->next;					//Faccio scorrere la testa
			free(canc);									//Dealloco l'elemento
		}
	}
	G->p = NULL;
	free(G->p);
	
	//cancello la lista di adiacenza
	VERTEX* canc2;
	for(i = 0; i < G->n; i++) {
		while(G->AdjList[i] != NULL) {
			canc2 = G->AdjList[i];						//Assegno a canc (puntatore d'appoggio)il valore dell' elemento
			G->AdjList[i] = G->AdjList[i]->next;		//Faccio scorrere la testa
			free(canc2);								//Dealloco l'elemento
		}
	}
	G->AdjList=NULL;									//Infine faccio puntare la testa a NULL
	free(G->AdjList);
	
	//cancello tutti gli altri elementi
    G->d = NULL;
    free(G->d);
    G->inPila = NULL;
	free(G->inPila);
    G->n = NIL;
}

/////////////////////////////////////


						/* LETTURA E ALLOCAZIONE GRAFO */
/*-------------------------------------------------------------------------------*/
GRAPH *GraphRead(char *file) {
	
	// Lettura di un grafo da file
    GRAPH *G ;
    VERTEX ** AdjList ;
    int n, i;
    if (( AdjList = AdjListRead (file ,&n)) == NULL ) return NULL ;
	G = GraphAlloc (n);
	for (i = 0; i < G->n; i++) {
		PADRI *nuovo = (PADRI*)malloc(sizeof(PADRI));
		if(nuovo == NULL) {
            printf("Non c'e' abbastanza memoria\n");
            system("pause");
            exit(1);
		}
		nuovo->id = 0;
		nuovo->next = NULL;
		G->p[i] = nuovo;
	}
    G->AdjList = AdjList ;
    return G;
}

/////////////////////////////////////

VERTEX ** AdjListRead ( char * file , int * n) {
    FILE *in = fopen (file,"r");
    VERTEX ** AdjList = NULL;
    int i ,j , dist ;
    if ( in == NULL ) return NULL ;
    if ( fscanf (in,"%d",n) == EOF ) return NULL ;
    
    AdjList = ( VERTEX **) malloc ((*n)* sizeof ( VERTEX *) );
    if(AdjList == NULL) {
		printf("Non c'e' abbastanza memoria\n");
		system("pause");
		exit(1);
    }
	
    VERTEX * tmpVertex = NULL;
	
	while ( fscanf (in ,"%d %d %d \n " ,&i ,&j ,&dist ) != EOF ) {
        tmpVertex = VertexAlloc (j, dist );
        tmpVertex -> next = AdjList [i];
        AdjList [i] = tmpVertex ;
    }
    fclose ( in );
    return AdjList ;
}

/////////////////////////////////////

VERTEX *VertexAlloc (int id, int dist ) {
    VERTEX *v = (VERTEX*)malloc(sizeof(VERTEX));
    if(v == NULL) {
		printf("Non c'e' abbastanza memoria\n");
		system("pause");
		exit(1);
	}
    v->id = id ;
    v->dist = dist ;
    v->next = NULL ;
    return v;
}

/////////////////////////////////////

GRAPH *GraphAlloc ( int n) {
    GRAPH *G = (GRAPH*)malloc(sizeof(GRAPH));
    if(G == NULL) {
		printf("Non c'e' abbastanza memoria\n");
		system("pause");
		exit(1);
	}
	
    G->n = n;
    G->AdjList = NULL;
	G->p = (PADRI**)malloc(n * sizeof(PADRI*));
	if(G->p == NULL) {
		printf("Non c'e' abbastanza memoria\n");
		system("pause");
		exit(1);
	}
	
    G->inPila = (int*)malloc(n*sizeof(int));
    if(G->inPila == NULL) {
		printf("Non c'e' abbastanza memoria\n");
		system("pause");
		exit(1);
	}
	
    G->d = (int*)malloc(n*sizeof(int));
    if(G->d == NULL) {
		printf("Non c'e' abbastanza memoria\n");
		system("pause");
		exit(1);
	}
    return G;
}

/////////////////////////////////////


									/* DIJKSTRA */
/*-------------------------------------------------------------------------------*/

void Dijkstra(GRAPH *G, int sorg, int *V) {
	Initialize(G, sorg, V);
	int nodo, primoAdj;
	VERTEX *temp = NULL;
	
	while(EmptyQueue(G, V) != NIL) {
        nodo = ExtractMin(G, V);
        V[nodo] = VUOTO;
		temp = G->AdjList[nodo];
		while(G->AdjList[nodo] != NULL) {
			primoAdj = G->AdjList[nodo]->id;
	        Relax(nodo, primoAdj, G);
	        G->AdjList[nodo] = G->AdjList[nodo]->next;
	    }
		G->AdjList[nodo] = temp;
	}
}

/////////////////////////////////////

void Initialize(GRAPH *G ,int sorg, int *V) {
	int nodo;
	
	// Inizializzazione di tutti i nodi
    for(nodo = 0; nodo < G->n; nodo++) {
        G->d[nodo] = INFINITO;
		G->p[nodo]->id = VUOTO;
		V[nodo] = nodo;
	}
	G->d[sorg] = 0;					// il nodo sorgente da se stesso dista 0
}

/////////////////////////////////////

int EmptyQueue(GRAPH* G, int V[]) {
	int i, vuoti = 0;
	
	//conta quanti nodi vuoti ci sono e li confronta col numero di nodi totale
	for(i = 0; i <G->n;i++) {
		if(V[i] == VUOTO)
			vuoti++;
	}
	
	//se vuoti = n nodi è vuota
	if(vuoti == G->n) return NIL;
	return INFINITO;
}

/////////////////////////////////////

int ExtractMin(GRAPH *G, int *V) {
    int i, min = INFINITO, indiceMin = 0;
    
    for(i = 0; i < G->n; i++) {
        if(V[i] > VUOTO && min > G->d[i]) {
            min = G->d[i];
            indiceMin = i;
        }
    }
    V[indiceMin] = VUOTO;
    return indiceMin;
}

/////////////////////////////////////

void Relax (int padre, int figlio, GRAPH *G) {
	
	if(G->d[figlio] > (G->d[padre] + G->AdjList[padre]->dist)) {		//se trova un cammino minore del precedente
		G->d[figlio] = G->d[padre] + G->AdjList[padre]->dist;			//imposta la distanza appena trovata
		
		//imposto come primo e unico padre del nodo il padre appena trovato
		if (G->p[figlio]->next == NULL) G->p[figlio]->id = padre;
		else {
			eliminaCorrispondenze(&G->p[figlio]);
			PADRI *temp = (PADRI*)malloc(sizeof(PADRI));
			if(temp == NULL) {
				printf("Non c'e' abbastanza memoria\n");
				system("pause");
				exit(1);
			}
			temp->id = padre;
			temp->next = NULL;
			G->p[figlio] = temp;
		}
	} else if (G->d[figlio] == (G->d[padre] + G->AdjList[padre]->dist)) {	//se trovo un cammino di peso uguale al precedente
		
		//aggiungo in testa il nuovo padre alla lista dei padri del nodo
		PADRI *temp = (PADRI*)malloc(sizeof(PADRI));
		if(temp == NULL) {
			printf("Non c'e' abbastanza memoria\n");
			system("pause");
			exit(1);
		}
		temp->id = padre;
		temp->next = G->p[figlio];
        temp->prec = NULL;
		G->p[figlio] = temp;
        G->p[figlio]->next->prec = G->p[figlio];
	}
}

/////////////////////////////////////

void eliminaCorrispondenze (PADRI **list) {
	//elimino tutti gli elementi della lista
	PADRI *temp = NULL;
	PADRI *elimina = (PADRI*)malloc(sizeof(PADRI));
	if(elimina== NULL) {
		printf("Non c'e' abbastanza memoria\n");
		system("pause");
		exit(1);
	}
	temp = *list;
	
	while (temp != NULL) {
		elimina = temp;
		free(elimina);
		temp = temp->next;
	}
}

/////////////////////////////////////

								/* STAMPA CAMMINI MINIMI */
/*-------------------------------------------------------------------------------*/

void stampaCamminiMinimi (GRAPH *G, int nodo, int sorg, int ultimoNodo, PILA **pila, int *flagScorrimento) {
    
	//caso base
	if (nodo == sorg) {
        printf("%d->", nodo);
        return;
    }
    stampaCamminiMinimi(G, G->p[nodo]->id, sorg, ultimoNodo, pila, flagScorrimento);
    
	//stampa nodo
	printf("%d", nodo);
    if (nodo != ultimoNodo) printf("->");			//stampa la freccia solo quando non si è arrivati all'ultimo nodo
    
	//controlli per push pop e scorrimento lista
    if (G->p[nodo]->next != NULL) {					//se ha più di un padre
        if (*flagScorrimento == FALSE) {			//se non è già stato fatto uno scorrimento risalendo la ricorsione
            if (G->inPila[nodo] == FALSE) {			//se il nodo non è pila
                push(pila, nodo, G->p[nodo]);		//push del nodo in pila
                G->inPila[nodo] = TRUE;
            }
            G->p[nodo] = G->p[nodo]->next;			//scorro la lista dei padri
            *flagScorrimento = TRUE;
        }
    } else if (G->inPila[nodo] == TRUE && *flagScorrimento == FALSE) {
        G->p[nodo] = pop(pila, nodo);				//pop dell'elemento
        G->inPila[nodo] = FALSE;
    }
}

/////////////////////////////////////

void push (PILA** testaPila, int nodo, PADRI* listaPadri) {
    PILA* tempPila = (PILA*)malloc(sizeof(PILA));	//inserisco il nodo in cima alla pila
    tempPila->id = nodo;
    tempPila->next = *testaPila;
    tempPila->vettPadri = listaPadri;
    *testaPila = tempPila;
}

/////////////////////////////////////

PADRI* pop (PILA** testaPila, int nodo) {
    if (*testaPila == NULL) {
        printf("Errore nella gestione della pila");
        return NULL;
    }
    PILA* temp = *testaPila;
    PILA* prec = *testaPila;
    while (temp->id != nodo) {						//cerco il nodo nella pila e lo elimino
        prec = temp;
        temp = temp->next;
    }
    if (prec == *testaPila) *testaPila = (*testaPila)->next;
    else prec->next = temp->next;
    PADRI* restituisci = temp->vettPadri;
    PILA* elimina = temp;
    free(elimina);
    return restituisci;
}

/////////////////////////////////////




