#!/bin/bash
#Needs to be run inside of a Repo to generate the Changelog for!
if [ "$#" -eq 0 ]
then
    echo "Usage: ./docgen.sh Tag Github_PAT"
    echo "Sample Usage: ./docgen.sh 4.2.0 Personal_access_token_with_REPO_Scope"
    exit 1
fi
if [ "$#" -eq 1 ]
then
    echo "Usage: ./docgen.sh Tag Github_PAT"
    echo "Sample Usage: ./docgen.sh 4.2.0 Personal_access_token_with_REPO_Scope"
    exit 1
fi
sudo npm install github-release-notes -g
gren changelog --generate --override --changelog-filename=RELEASE_NOTES_Features.md --repo=mwc-qt-wallet --username=mwcproject --token=$2
gren changelog --generate --override --changelog-filename=RELEASE_NOTES_Commits.md -D commits -c ./.docgen/.changelog_config.js -C "Autogenerate Changelog"  --repo=mwc-qt-wallet --username=mwcproject --token=$2
#Combine the 2 styles and add some formatting for the final .MD file
echo -e "There are 2 types of Release Notes availlable:\n<details>\n  <summary>Issue-based</summary>\n\n" > start.md
echo -e  "</details>\n<details>\n  <summary>Commit-based</summary>\n\n" > seperator.md
echo -e  "</details>\n" > end.md
ls
mkdir -p ./DOC/release_notes/
cat start.md RELEASE_NOTES_Features.md seperator.md RELEASE_NOTES_Commits.md end.md > ./DOC/release_notes/Release_notes_V$1.md
sudo rm start.md && sudo rm seperator.md && sudo rm end.md && sudo rm RELEASE_NOTES_Commits.md && sudo rm RELEASE_NOTES_Features.md
