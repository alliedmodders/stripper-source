import re
import os, sys
import subprocess

argv = sys.argv[1:]
if len(argv) < 2:
	sys.stderr.write('Usage: generate_headers.py <source_path> <output_folder>\n')
	sys.exit(1)

SourceFolder = os.path.abspath(os.path.normpath(argv[0]))
OutputFolder = os.path.normpath(argv[1])

class FolderChanger:
	def __init__(self, folder):
		self.old = os.getcwd()
		self.new = folder

	def __enter__(self):
		if self.new:
			os.chdir(self.new)

	def __exit__(self, type, value, traceback):
		os.chdir(self.old)

def run_and_return(argv):
	# Python 2.6 doesn't have check_output.
	if hasattr(subprocess, 'check_output'):
		text = subprocess.check_output(argv)
		if str != bytes:
			text = str(text, 'utf-8')
	else:
		p = subprocess.Popen(argv, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		output, ignored = p.communicate()
		rval = p.poll()
		if rval:
			raise subprocess.CalledProcessError(rval, argv)
		text = output.decode('utf8')
	return text.strip()

def get_git_version():
	revision_count = run_and_return(['git', 'rev-list', '--count', 'HEAD'])
	revision_hash = run_and_return(['git', 'log', '--pretty=format:%h:%H', '-n', '1'])
	shorthash, longhash = revision_hash.split(':')

	return revision_count, shorthash, longhash

def output_version_header():
	with FolderChanger(SourceFolder):
		count, shorthash, longhash = get_git_version()
	
	with open(os.path.join(SourceFolder, 'product.version')) as fp:
		contents = fp.read()
	m = re.match('(\d+)\.(\d+)\.(\d+)(.*)', contents)
	if m == None:
		raise Exception('Could not detremine product version')
	major, minor, release, tag = m.groups()
	product = "{0}.{1}.{2}".format(major, minor, release)

	with open(os.path.join(OutputFolder, 'stripper_version_auto.h'), 'w') as fp:
		fp.write("""
#ifndef _STRIPPER_AUTO_VERSION_INFORMATION_H_
#define _STRIPPER_AUTO_VERSION_INFORMATION_H_

#define STRIPPER_BUILD_STRING 		\"{0}\"
#define STRIPPER_BUILD_UNIQUEID		\"{1}:{2}\" STRIPPER_BUILD_STRING
#define STRIPPER_FULL_VERSION		\"{3}.{4}.{5}\" STRIPPER_BUILD_STRING
#define STRIPPER_FILE_VERSION		{3},{4},{5},0

#endif /* _STRIPPER_AUTO_VERSION_INFORMATION_H_ */
		""".format(tag, count, shorthash, major, minor, release))

output_version_header()