# SayVox - Vocal Style (VST3 pour FL Studio et Cubase)

Plugin de mixage vocal "prêt à poser" : 169 presets (styles d'artistes du monde entier + presets Pro),
4 rôles (Lead / Doubles / Backs / Adlibs), interface simple à 7 potards.

IMPORTANT : les presets sont des reconstitutions APPROXIMATIVES du style de chaque artiste
(EQ, compression, grain, brillance, écho, espace, largeur). Ce ne sont pas des copies exactes.
Pour vendre le plugin, renomme les presets en "style inspiré de..." ou en noms de style.

## Ce que fait la chaîne (dans l'ordre)
Entrée -> filtre graves 24 dB -> de-esser -> EQ anti-boue + corps -> compresseur doux ->
saturation (grain) -> présence + air -> compresseur rapide anti-crêtes ->
largeur (doubler mono-compatible) -> écho ping-pong synchronisé au tempo -> reverb -> sortie
(avec écrêtage doux de sécurité).

Latence : ZÉRO (aucun look-ahead, aucun sur-échantillonnage).

## Les réglages
- Menu des presets (flèches < >) : cliquer sur le nom pour choisir par catégorie.
- Rôle : Lead, Doubles (plus sombre, plus large, -4 dB), Backs (encore plus larges, -5 dB), Adlibs (brillants + échos).
- Potards : Compression, Grain, Brillance, Écho, Espace, Largeur.
- Intensité : dose tout le preset (0 % = voix quasi intacte, 100 % = preset, 150 % = poussé).
- Entrée / Sortie : gains. Écho sync : 1/4, 1/8, 1/8 pointée, 1/16.
- Double-clic sur un potard : retour à sa valeur par défaut.

## Compiler le plugin SANS rien installer (GitHub Actions)

Étape 1 - Créer un dépôt GitHub (privé) et y envoyer ce dossier.
   Confirmation : tu vois les dossiers Source, tools, .github et le fichier CMakeLists.txt sur GitHub.

Étape 2 - Ouvrir l'onglet "Actions" du dépôt.
   Confirmation : le workflow "Build SayVox VST3 (Windows)" est listé.
   Clique dessus puis "Run workflow" (si rien ne démarre tout seul).

Étape 3 - Attendre 8 à 15 minutes (première compilation, JUCE est téléchargé).
   Confirmation : une coche verte apparaît.

Étape 4 - En bas de la page du run, télécharger "SayVox-VST3-Windows" (fichier .zip) et le décompresser.
   Confirmation : tu obtiens un dossier "SayVox.vst3".

Étape 5 - Copier le dossier SayVox.vst3 dans :
       C:\Program Files\Common Files\VST3\
   Confirmation : le dossier est bien visible dans cet emplacement.

Étape 6 - FL Studio : Options > Manage plugins > "Find installed plugins" (scan).
   Confirmation : SayVox apparaît dans la liste. Place-le sur une piste Mixer (slot d'effet).

Étape 7 - Cubase : Studio > Gestion des plug-ins VST > "Mettre à jour" (ou redémarrer Cubase).
   Confirmation : SayVox est dans la liste. Insère-le sur ta piste voix.

Étape 8 - Test : choisis le preset "Pro - Studio Clean", rôle Lead, joue la voix.
   Confirmation : les vumètres IN / OUT bougent et le son change quand tu bouges un potard.

## Modifier ou ajouter des artistes
1. Ouvre tools/gen_presets.py, ajoute une ligne dans ARTISTS :
       ("Nom Artiste", "archétype", dict(comp=+5, bright=-3))
   (archétypes : clean, melodic, trap, forward, boombap, drill, grime, rnb, pop, fr, frmel, rage, latin, afro, emo, poprap, arab)
2. Lance : python tools/gen_presets.py
3. Envoie sur GitHub : le plugin est recompilé automatiquement.

## Compiler en local (optionnel)
Installer Visual Studio 2022 (charge "Développement Desktop en C++") et CMake, puis :
    cmake -B build -A x64
    cmake --build build --config Release --target SayVox_VST3
Résultat : build\SayVox_artefacts\Release\VST3\SayVox.vst3

## Licence JUCE
Le plugin utilise JUCE 8. Usage perso / gratuit sous conditions ; pour vendre le plugin, vérifie la licence JUCE
(juce.com/juce-8-licence).
