#!/usr/bin/env python
#################################################################################
# SPIFFS does not support directories. To get around this, this script will     #
# take the os.sep and replace it with '+++'. Http handlers for the ESP32 will   #
# take this in to account when serving file. It will be transparent to the      #
# http requests.                                                                #
#                                                                               #
# Additionally, the html, css and js files will be minified.                    #
#################################################################################

import os
import shutil
from css_html_js_minify import process_single_html_file, process_single_js_file, process_single_css_file#, html_minify, js_minify, css_minify

def main():
    rootdir = './main/web'
    for subdir, dirs, files in os.walk(rootdir):

        for file in files:
            if ((os.sep + "." in subdir) or (os.sep + "." in dirs)):
                continue
            sourceFile = os.path.join(subdir, file)
##            spiffsFile = os.path.join("build", subdir, file)
            spiffsFile = os.path.join(subdir.replace("main", "build"), file)
##            if(not os.path.exists(os.path.join("build", subdir))):
##                os.mkdir(os.path.join("build", subdir))

            if(not os.path.exists(os.path.join("..","build"))):
                os.mkdir(os.path.join("..","build"))
            if(not os.path.exists(subdir.replace("main", "build"))):
                os.mkdir(subdir.replace("main", "build"))

            shutil.copy(sourceFile, spiffsFile)
            fileName, fileExtension = os.path.splitext(spiffsFile)
            if (fileExtension == ".html"):
                process_single_html_file(spiffsFile, overwrite=True)
            elif (fileExtension == ".css"):
                process_single_css_file(spiffsFile, overwrite=True)
            elif (fileExtension == ".js"):
                process_single_js_file(spiffsFile, overwrite=True)




# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    main()