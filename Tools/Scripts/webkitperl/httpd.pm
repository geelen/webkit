# Copyright (C) 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved
# Copyright (C) 2006 Alexey Proskuryakov (ap@nypop.com)
# Copyright (C) 2010 Andras Becsi (abecsi@inf.u-szeged.hu), University of Szeged
# Copyright (C) 2011 Research In Motion Limited. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
# 3.  Neither the name of Apple Inc. ("Apple") nor the names of
#     its contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Module to share code to start and stop the Apache daemon.

use strict;
use warnings;

use File::Copy;
use File::Path;
use File::Spec;
use File::Spec::Functions;
use IPC::Open2;

use webkitdirs;

BEGIN {
   use Exporter   ();
   our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);
   $VERSION     = 1.00;
   @ISA         = qw(Exporter);
   @EXPORT      = qw(&getHTTPDPath
                     &hasHTTPD
                     &getHTTPDConfigPathForTestDirectory
                     &getDefaultConfigForTestDirectory
                     &openHTTPD
                     &closeHTTPD
                     &setShouldWaitForUserInterrupt);
   %EXPORT_TAGS = ( );
   @EXPORT_OK   = ();
}

my $tmpDir = "/tmp";
my $httpdPidDir = File::Spec->catfile($tmpDir, "WebKit");
my $httpdPidFile = File::Spec->catfile($httpdPidDir, "httpd.pid");
my $httpdPid;
my $waitForUserInterrupt = 0;
my $waitBeginTime;
my $waitEndTime;

$SIG{'INT'} = 'handleInterrupt';
$SIG{'TERM'} = 'handleInterrupt';

sub getHTTPDPath
{
    my $httpdPath;
    if (isDebianBased()) {
        $httpdPath = "/usr/sbin/apache2";
    } else {
        $httpdPath = "/usr/sbin/httpd";
    }
    return $httpdPath;
}

sub hasHTTPD
{
    my @command = (getHTTPDPath(), "-v");
    return system(@command) == 0;
}

sub getApacheVersion
{
    my $httpdPath = getHTTPDPath();
    my $version = `$httpdPath -v`;
    $version =~ s/.*Server version: Apache\/(\d+\.\d+).*/$1/s;
    return $version;
}

sub getDefaultConfigForTestDirectory
{
    my ($testDirectory) = @_;
    die "No test directory has been specified." unless ($testDirectory);

    my $httpdConfig = getHTTPDConfigPathForTestDirectory($testDirectory);
    my $documentRoot = "$testDirectory/http/tests";
    my $jsTestResourcesDirectory = $testDirectory . "/resources";
    my $mediaResourcesDirectory = $testDirectory . "/media";
    my $typesConfig = "$testDirectory/http/conf/mime.types";
    my $httpdLockFile = File::Spec->catfile($httpdPidDir, "httpd.lock");
    my $httpdScoreBoardFile = File::Spec->catfile($httpdPidDir, "httpd.scoreboard");

    my @httpdArgs = (
        "-f", "$httpdConfig",
        "-C", "DocumentRoot \"$documentRoot\"",
        # Setup a link to where the js test templates are stored, use -c so that mod_alias will already be loaded.
        "-c", "Alias /js-test-resources \"$jsTestResourcesDirectory\"",
        "-c", "Alias /media-resources \"$mediaResourcesDirectory\"",
        "-c", "TypesConfig \"$typesConfig\"",
        # Apache wouldn't run CGIs with permissions==700 otherwise
        "-c", "User \"#$<\"",
        "-c", "PidFile \"$httpdPidFile\"",
        "-c", "ScoreBoardFile \"$httpdScoreBoardFile\"",
    );

    if (getApacheVersion() eq "2.2") {
        push(@httpdArgs, "-c", "LockFile \"$httpdLockFile\"");
    }

    # FIXME: Enable this on Windows once <rdar://problem/5345985> is fixed
    # The version of Apache we use with Cygwin does not support SSL
    my $sslCertificate = "$testDirectory/http/conf/webkit-httpd.pem";
    push(@httpdArgs, "-c", "SSLCertificateFile \"$sslCertificate\"") unless isCygwin();

    return @httpdArgs;

}

sub getHTTPDConfigPathForTestDirectory
{
    my ($testDirectory) = @_;
    die "No test directory has been specified." unless ($testDirectory);

    my $httpdConfig;
    my $httpdPath = getHTTPDPath();
    my $httpdConfDirectory = "$testDirectory/http/conf/";
    my $apacheVersion = getApacheVersion();

    if (isCygwin()) {
        my $libPHP4DllPath = "/usr/lib/apache/libphp4.dll";
        # FIXME: run-webkit-tests should not modify the user's system, especially not in this method!
        unless (-x $libPHP4DllPath) {
            copy("$httpdConfDirectory/libphp4.dll", $libPHP4DllPath);
            chmod(0755, $libPHP4DllPath);
        }
        $httpdConfig = "cygwin-httpd.conf";  # This is an apache 1.3 config.
    } elsif (isDebianBased()) {
        $httpdConfig = "debian-httpd-$apacheVersion.conf";
    } elsif (isFedoraBased()) {
        $httpdConfig = "fedora-httpd-$apacheVersion.conf";
    } else {
        # All other ports use apache2, so just use our default apache2 config.
        $httpdConfig = "apache$apacheVersion-httpd.conf";
    }
    return "$httpdConfDirectory/$httpdConfig";
}

sub openHTTPD(@)
{
    my (@args) = @_;
    die "No HTTPD configuration has been specified" unless (@args);
    mkdir($httpdPidDir, 0755);
    die "No write permissions to $httpdPidDir" unless (-w $httpdPidDir);

    if (-f $httpdPidFile) {
        open (PIDFILE, $httpdPidFile);
        my $oldPid = <PIDFILE>;
        chomp $oldPid;
        close PIDFILE;
        if (0 != kill 0, $oldPid) {
            print "\nhttpd is already running: pid $oldPid, killing...\n";
            if (!killHTTPD($oldPid)) {
                cleanUp();
                die "Timed out waiting for httpd to quit";
            }
        }
        unlink $httpdPidFile;
    }

    my $httpdPath = getHTTPDPath();

    open2(">&1", \*HTTPDIN, $httpdPath, @args);

    my $retryCount = 20;
    while (!-f $httpdPidFile && $retryCount) {
        sleep 1;
        --$retryCount;
    }

    if (!$retryCount) {
        cleanUp();
        die "Timed out waiting for httpd to start";
    }

    $httpdPid = <PIDFILE> if open(PIDFILE, $httpdPidFile);
    chomp $httpdPid if $httpdPid;
    close PIDFILE;

    waitpid($httpdPid, 0) if ($waitForUserInterrupt && $httpdPid);

    return 1;
}

sub closeHTTPD
{
    close HTTPDIN;
    my $succeeded = killHTTPD($httpdPid);
    cleanUp();
    unless ($succeeded) {
        print STDERR "Timed out waiting for httpd to terminate!\n" unless $succeeded;
        return 0;
    }
    return 1;
}

sub killHTTPD
{
    my ($pid) = @_;

    return 1 unless $pid;

    kill 15, $pid;

    my $retryCount = 20;
    while (kill(0, $pid) && $retryCount) {
        sleep 1;
        --$retryCount;
    }
    return $retryCount != 0;
}

sub setShouldWaitForUserInterrupt
{
    $waitForUserInterrupt = 1;
}

sub handleInterrupt
{
    # On Cygwin, when we receive a signal Apache is still running, so we need
    # to kill it. On other platforms (at least Mac OS X), Apache will have
    # already been killed, and trying to kill it again will cause us to hang.
    # All we need to do in this case is clean up our own files.
    if (isCygwin()) {
        closeHTTPD();
    } else {
        cleanUp();
    }

    print "\n";
    exit(1);
}

sub cleanUp
{
    rmdir $httpdPidDir;
}
