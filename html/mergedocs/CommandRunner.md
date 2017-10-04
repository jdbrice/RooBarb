#<cldoc:jdb::CommandRunner>

Runs a generic shell command in a config aware env


Example config for generating a filelist:
``` xml

<?xml version="1.0" encoding="UTF-8"?>
<config>

	<Task name="flm" type="CommandRunner" config="" nodePath="muonFLM" />
	<Logger color="true" globalLogLevel="info" />
	
	<R url="../Common/{R:active}/" active="Run14_AuAu_200_Mtd" />
	<Include url="{R:url}/cwd.xml" />
	<muonFLM>
		<Logger color="true" globalLogLevel="trace" />

		<output>
			<File url="{CWD:out}/filelists/SameEventMuMuPairs.lis" />
		</output>

		<command>ls -1 {CWD:out}/SameEventMuMuPairs/*.root &gt; {CWD:out}/filelists/SameEventMuMuPairs.lis</command>
	</muonFLM>

</config>

```


~~~~xml

<?xml version="1.0" encoding="UTF-8"?>
<config>

	<Task name="flm" type="CommandRunner" config="" nodePath="muonFLM" />
	<Logger color="true" globalLogLevel="info" />
	
	<R url="../Common/{R:active}/" active="Run14_AuAu_200_Mtd" />
	<Include url="{R:url}/cwd.xml" />
	<muonFLM>
		<Logger color="true" globalLogLevel="trace" />

		<output>
			<File url="{CWD:out}/filelists/SameEventMuMuPairs.lis" />
		</output>

		<command>ls -1 {CWD:out}/SameEventMuMuPairs/*.root &gt; {CWD:out}/filelists/SameEventMuMuPairs.lis</command>
	</muonFLM>

</config>

~~~~