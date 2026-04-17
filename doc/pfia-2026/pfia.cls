%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Style des articles pour PFIA
% Auteur : Dominique.Longin@irit.fr
% Date : 05/12/2019
% Version : V1.0
% Modification : création
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Date : 05/05/2022
% Version : V2.1
% Modification :  reprise du style
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Date : 03/11/2022
% Version : V2.2
% Modification : mise à jour nouvelle
%                longueur des articles
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Date : 20/07/2023
% Version : V2.3
% Modification : largeur page 1,2 -> 1,7cm
%                (valeur réelle !)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Date : 07/03/2024
% Version : V2.4
% Modification : demande d'entrer le prénom
%                des auteurs EN ENTIER
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Date : 26/11/2024
% Version : V2025
% Modification : ajout modalités dépôt HAL
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{pfia}

\usepackage{ifthen}

% numéro de version du style
\newcommand{\version}{V.~\arabic{anneePFIA}}

% option permettant de savoir si on est en French
\newboolean{frenchEnabled}
\setboolean{frenchEnabled}{false}

% option permettant d'afficher l'aide à la saisie dans HAL
\newboolean{halEnabled}
\setboolean{halEnabled}{false}

\DeclareOption{french}{%
    \typeout{Option FRENCH active}%
    \setboolean{frenchEnabled}{true}%
}

\DeclareOption{dochal}{%
    \typeout{Option DOCHAL active}%
    \setboolean{halEnabled}{true}%
}

\DeclareOption*{\PassOptionsToClass{\CurrentOption}{article}}

\ProcessOptions


\ifthenelse{\boolean{frenchEnabled}}{%
    \LoadClass[10pt,twocolumn,french]{article}%
    \newcommand{\quotes}[1]{\og~#1\fg}
}{
    \LoadClass[10pt,twocolumn]{article}%
    \newcommand{\fup}[1]{\textsuperscript{#1}}%
    \newcommand{\quotes}[1]{``#1''}%
}

% ---------------------------------
% PACKAGES
% ---------------------------------

\usepackage[T1]{fontenc}
\usepackage[utf8]{inputenc}
\usepackage{calc}
\usepackage{xspace}
\usepackage{times}
\usepackage{geometry}
\usepackage{titlesec}
\usepackage[pdftex,bookmarks]{hyperref}
\usepackage{babel}

% ---------------------------------
% LAYOUT
% ---------------------------------

% configuration package hyperref
\hypersetup{
    linktocpage=true,%
    frenchlinks=false,
    %%--- Color Links ---
    colorlinks=true,%
    linkcolor=afiablue,%
    citecolor=afialightblue,%
    urlcolor=afiablue,%
    menucolor=afiablue,%
    %%--- Doc Info ---
    pdftitle={Conférence hébergée de l'AFIA},%
    pdfauthor={AFIA},%
}


\flushbottom

\setlength{\baselineskip}{12pt}

% taille du papier et de la zone de texte
% hauteur contient un nombre entier de lignes
\geometry{a4paper,text={17.6cm,58\baselineskip},centering}

% espacement entre les colonnes
\setlength{\columnsep}{1cm}

% pas d'indentation des paragraphes
\setlength{\parindent}{0em}

% style des pages
\pagestyle{empty}

% gestion automatique des dépassements dans les marges
\sloppy

% ---------------------------------
% INSERTS
% ---------------------------------

% espacement des sections
\titlespacing{\section}% command
{0em}% left
{*1}% before
{*1}% after
[0em]% right-sep margin increasing

% espacement des sous-sections
\titlespacing{\subsection}% command
{0em}% left
{*1}% before
{*0.7}% after
[0em]% right-sep margin increasing

% espacement des sous-sous-sections
\titlespacing{\subsubsection}% command
{0em}% left
{*0.8}% before
{*0.5}% after
[0em]% right-sep margin increasing

% espacement des sous-sous-sections
\titlespacing{\paragraph}% command
{0em}% left
{*0.5}% before
{*0.5}% after
[0em]% right-sep margin increasing

% résumé en français
\newenvironment{resume}{%
    \thispagestyle{empty}%
    \subsection*{Résumé}\em}{}

% mots-clés en français
\newenvironment{motscles}{\subsection*{Mots-clés}\em}{}

% résumé en anglais
\renewenvironment{abstract}{\subsection*{Abstract}\em}{}

% mots-clés en français
\newenvironment{keywords}{\subsection*{Keywords}\em}{}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% MACROS
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% CHAMPS A METTRE A JOUR CHAQUE ANNEE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% année de PFIA
\newcounter{countAnneePFIA}
\setcounter{countAnneePFIA}{2026}
\newcommand{\anneePFIA}{\arabic{countAnneePFIA}\xspace}
% lieu de PFIA
\newcommand{\lieuPFIA}{Dijon\xspace}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


\ifthenelse{\boolean{halEnabled}}{%

    % nombre de CH construites
    \newcounter{countCH}
    \setcounter{countCH}{0}
    
    % [#1] : valeur par défaut = OFF. Si valeur = ON alors l'écriture de cette CH sera fusionnée avec la suivante
    % #2 : acronyme CH (ex. : APIA)
    % #3 : N° édition CH en 2023 (ex. pour APIA : 9)
    % #4 : libellé complet de la CH (ex. APIA : \fup{e} conférence nationale sur les Applications Pratiques de ...)
    \newcommand{\CHbuilder}[4][OFF]{%
        % on incrémente le compteur général de CH
        \stepcounter{countCH}%
        % on sauvegarde s'il faut fusionner cette CH avec la suivante
        \expandafter\def\csname chAfusionner\alph{countCH}\endcsname{#1}%
        % on sauvegarde l'acronyme
        \expandafter\def\csname chAcronyme\alph{countCH}\endcsname{#2}%
        % on calcule le numéro de l'édition de la CH par rapport à 2023
        \newcounter{countThisCH\alph{countCH}}
        \setcounter{countThisCH\alph{countCH}}{#3+\value{countAnneePFIA}-2023}%
        % on sauvegarde le libellé de la CH
        \expandafter\def\csname chLibelle\alph{countCH}\endcsname{#4}%
    }
    
    % écriture de la liste des libellés de CH
    \newcommand{\writeCHs}{%
        % compteur intermédiaire
        \newcounter{countTmp}
        \setcounter{countTmp}{0}
        \begin{itemize}
        \whiledo{\value{countTmp}<\value{countCH}}{%
            \stepcounter{countTmp}%
            \item \arabic{countThisCH\alph{countTmp}}\csname chLibelle\alph{countTmp}\endcsname\ 
            \ifthenelse{\equal{\csname chAfusionner\alph{countTmp}\endcsname}{ON}}{%
                \stepcounter{countTmp}%
                et \arabic{countThisCH\alph{countTmp}}\csname chLibelle\alph{countTmp}\endcsname\ 
                (\addtocounter{countTmp}{-1}\csname chAcronyme\alph{countTmp}\endcsname-\addtocounter{countTmp}{1}\csname chAcronyme\alph{countTmp}\endcsname~\anneePFIA)
            }{%
                (\csname chAcronyme\alph{countTmp}\endcsname~\anneePFIA)
            }
        }
        \end{itemize}
    }
    
    \CHbuilder{APIA}%
        {9}%
        {\fup{e} conférence nationale sur les Applications Pratiques de l'Intelligence Artificielle}
    
    \CHbuilder{CAp}%
        {25}%
        {\fup{e} Conférence sur l'Apprentissage automatique}
    
    \CHbuilder{CNIA}%
        {26}%
        {\fup{e} Conférence Nationale en Intelligence Artificielle}
    
    \CHbuilder{IC}%
        {34}%
        {\fup{es} journées francophones d'Ingénierie des Connaissances}
    
    \CHbuilder{JFPC}%
        {18}%
        {\fup{es} Journées Francophones de Programmation par Contraintes}
    
    \CHbuilder{JFSMA}%
        {31}%
        {\fup{es} Journées Francophones sur les Systèmes Multi-Agents}
    
    \CHbuilder[ON]{JIAF}%
        {17}%
        {\fup{es} Journées d'Intelligence Artificielle Fondamentale}
    
    \CHbuilder{JFPDA}%
        {18}%
        {\fup{es} Journées Francophones sur la Planification, la Décision et l’Apprentissage pour la conduite de systèmes}
    
    \CHbuilder{RJCIA}%
        {21}%
        {\fup{es} Rencontres des Jeunes Chercheurs en Intelligence Artificielle}
    
    \CHbuilder{SFC}%
        {28}%
        {\fup{es} rencontres de la Société Francophone de Classification}
    
    \AtEndDocument{
    
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        \newpage \appendix

        \begin{quote}
            \color{red} Les recommandations suivantes peuvent être supprimées ou affichées ci-dessous en désactivant ou en activant respectivement l'option \textsf{dochal} dans la classe de document.
        \end{quote}
        
        \section{Recommandations pour saisir votre publication dans HAL}
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        
        
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        \subsection{Pourquoi des recommandations ?}
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        
        Les recommandations suivantes :
        \begin{itemize}
            \item assureront une homogénéité dans HAL des notices des articles acceptés à des conférences hébergées (CH) de PFIA ;
                
            \item vous indiquent la bonne valeur des différents champs à remplir, garantissant ainsi le référencement correct de votre article et la possibilité pour la communauté de le retrouver facilement sur HAL.
                
        \end{itemize}
        Aussi, il est \textbf{fortement} recommandé de suivre les étapes suivantes. Dans ce qui suit, on considère que vous avez déjà un compte HAL ainsi qu'un idHal, et que vous êtes connectés à votre compte.
        
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        \subsection{Note importante}
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        
        L'AFIA est engagée dans une démarche Sciences Ouvertes, et met à disposition ses publications dans l'archive HAL (voir la collection principale \href{https://hal.science/AFIA}{https://hal.science/AFIA}). En tant qu'auteur, vous pouvez contribuer à cette démarche en déposant votre article dans HAL. 
        %
        Ce dépôt n'est bien sûr pas obligatoire et est fait sous votre entière responsabilité.
        %
        Enfin, assurez-vous avant tout dépôt que vos co-auteurs ne déposent pas eux-mêmes une version du même article dans HAL.
        %
        N'oubliez pas que l'AFIA ne réclame aucune exclusivité des contenus et que vous restez entièrement propriétaire de votre publication.
        
        Enfin, \textbf{\textcolor{afiablue}{les numéros de paragraphe ci-dessous font référence à l'ordre dans lequel les champs vous sont demandés par HAL}} afin de vous rendre le suivi des champs à renseigner plus facile. (Donc, ne vous étonnez pas s'ils n'apparaissent pas dans l'ordre.)
        
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        \subsection{Champs HAL importants pour l'AFIA}
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        
        Cette section ne reprend que les champs particulièrement importants pour que votre publication soit clairement visible et identifiée comme étant associée à l'AFIA. (Pour les autres champs, se référer à la section suivante en cas de besoin.)

        Tout d'abord, il s'agit de créer un nouveau dépôt HAL en choisissant \textbf{Communication à un congrès} comme type de dépôt. 
        
        \paragraph{11.\quad Titre du congrès.}
        
        Traditionnellement, l'édition de la CH est associée à son titre. Pour \anneePFIA, les intitulés sont à choisir parmi : \writeCHs
        
        \paragraph{12.\quad Organisateur du congrès.}
        
        La valeur à indiquer ici est \val{Plate-Forme Intelligence Artificielle}. (Attention à la casse des caractères.) 
        
        \paragraph{13.\quad Date début congrès.} Il s'agit ici de la date de début de la CH à laquelle votre article a été accepté (qui peut être différente de la date de début de PFIA). 
        
        \paragraph{14.\quad Date fin congrès.} De même, date de fin de la CH de votre article (qui peut être différente de la date de fin de PFIA).
        
        \paragraph{15.\quad Ville et Pays.} Bien sûr, \field{Ville} = \val{\lieuPFIA} et \field{Pays} = \val{France}.
        
        \paragraph{16.\quad Actes.} Il convient de mettre la valeur \val{Oui}.
        
        \paragraph{17.\quad Date de publication.} Considérez uniquement l'année et le mois. Typiquement, la valeur devrait être \val{\anneePFIA-07}.
        
        \paragraph{18.\quad \'Editeur commercial.} La valeur à indiquer ici est \linebreak \val{Association Française pour l'Intelligence Artificielle}. 
        
        (Attention à bien respecter la casse des caractères.)
        
        \paragraph{22.\quad Référence interne.} Vous devez entrer ici deux valeurs (en cliquant que \quotes{+ Ajouter} après chaque valeur) : \val{AFIA}, puis \val{PFIA-ART-<acronyme CH>}
        
        
        
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        \subsection{Autres champs HAL}
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        
        Si vous êtes peu familiers de HAL, ou souhaitez vous assurer que les métadonnées de votre publication sont correctes, vous pouvez vous référer aux suggestions ci-dessous.
        
        \paragraph{1.\quad Choix du type de document.}
        
        Il faut sélectionner \val{Communication dans un congrès}. 
        
        \paragraph{2.\quad Dépôt du fichier de la publication et informations.} 
        
        Il convient donc de cliquer-déposer votre fichier PDF dans l'espace réservé.
        
        N'oubliez pas ensuite de donner une valeur aux quatre champs associés au fichier déposé :
        \begin{itemize}
          \item \field{Origine fichier principal} = \val{Les fichiers que je dépose sont des fichiers produits par l'(les) auteur(s)} ;
              
          \item \field{Format} = \val{Document PDF} (préféré) ;
          
          \item \field{Visibilité} = \val{Immédiatement} (à défaut, précisez le délai d'embargo à l'issu duquel la publication pourra être rendue visible) ;
          
          \item \field{Licence} = \val{CC BY NC ND} (ou toute autre licence qui vous convient mieux).
        
        \end{itemize}
        
        \paragraph{3.\quad Auteurs.}
        
        Parfois HAL reconnait depuis le fichier PDF la liste des auteurs. Vérifiez-la et, le cas échéant, complétez-la.
        En entrant le prénom puis les premières lettres du noms, HAL devrait être en mesure de vous proposer une liste d'auteurs.
        
        Si plusieurs occurrences du même auteur apparaissent, préférez la référence en vert (plus récente).
        
        Si un des auteurs n'a jamais déposé dans HAL, il se peut qu'il soit inconnu. Il convient alors de remplir à la main son affiliation : il est fortement conseillé dans ce cas de demander à l'auteur lui-même quoi mettre. 
        
        Attention : l'ordre des auteurs doit être celui apparaissant sur la publication. 
        
        \paragraph{4.\quad Titre.}
        
        Il s'agit du titre de votre publication. Pensez à sélectionner \textbf{d'abord} la langue associée au titre, puis une fois le titre saisi, cliquer sur \quotes{+ Ajouter}.
        
        \paragraph{5.\quad Domaine.}
        
        En principe, vous devriez sélectionner la valeur \val{Informatique / Intelligence artificielle [cs.AI]}.
        
        \paragraph{6.\quad Autres disciplines du document.}
        
        Vous pouvez saisir ici d'autres domaines du document.
        
        \paragraph{7.\quad Résumé.}
        
        Il n'est pas obligatoire mais peut influencer les résultats d'une recherche dans HAL. Attention à sélectionner la langue dans laquelle le résumé est rédigé puis cliquer sur \quotes{+ Ajouter} si HAL n'avait pas déjà rempli ce champs. 
        
        \paragraph{8.\quad Mots-clés.}
        
        Comme le résumé, ils ne sont pas obligatoires mais peuvent rendre plus efficace une recherche de votre document. Cliquer sur \quotes{+ Ajouter} après chaque mot-clé (un seul par champs !).
        
        \paragraph{9.\quad Langue du document.}
        
        \val{Français} ou \val{Anglais} selon votre cas.
        
        \paragraph{10.\quad Licence.}
        
        Il s'agit ici de la licence de la notice (les données décrivant votre article). Là encore, il est recommandé de mettre là encore \val{CC BY NC ND} (ou toute autre licence qui vous convient mieux).
        
        \paragraph{19.\quad Vulgarisation.} \val{Non}.
        
        \paragraph{20.\quad Comité de lecture.} \val{Oui}.
        
        \paragraph{21.\quad Audience.} \val{Nationale}.
    }   
}{%
}