#!/bin/bash

# Set the optinos
includeDir="../Testlab/include"
buildDir="../Testlab/x64"
installDir="./lib/testlab"
keyword="\-cpp"

# Remove previously installed builds
echo "Cleaning the installation directory..."
if [ -d "$installDir" ]; then
	files=($(ls $installDir -I CMakeLists.txt))
	if [ -n "$files" ]; then
		files=("${files[@]/#/$installDir/}")
		rm -r "${files[@]}"
	fi
else
	mkdir $installDir
fi

# Copy all the builds
names=($(ls $buildDir))
for name in "${names[@]}"; do
	# Create the destination directory
	targetName="$name"
	targetDir="$installDir/$name"
	mkdir $targetDir
	echo "Installing $targetName"
	
	# Copy compiled files
	find "$buildDir/$name" -maxdepth 1 -type f -name "testlab-*" -exec cp "{}" $targetDir \;
done
find $includeDir -type f -exec cp "{}" $installDir \;
echo "Installation completed"
