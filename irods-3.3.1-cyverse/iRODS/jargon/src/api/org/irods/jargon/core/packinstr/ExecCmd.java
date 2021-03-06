/**
 * 
 */
package org.irods.jargon.core.packinstr;

import org.irods.jargon.core.exception.JargonException;

import edu.sdsc.grid.io.irods.Tag;

/**
 * Immutable object gives translation of an ExecCmd operation into XML. This is
 * the operation for remote execution protocol format.
 * <p/>
 * Note that this packing instruction must deal with backwards compatability for
 * versions of ExecCmd prior to the addition of enhanced streaming of large
 * result sets. The method names indicate which version of iRODS the packing
 * instruction will be for.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */

public final class ExecCmd extends AbstractIRODSPackingInstruction {

	public static final String PI_TAG = "ExecCmd_PI";
	public static final String PI_TAG_BACKWORD_COMPATABLE = "ExecCmd241_PI";

	public static final int STANDARD_EXEC_ENCAPSULATE_DATA_IN_RESPONSE_API_NBR = 634;
	public static final int EXEC_AND_USE_ENHANCED_STREAM = 692;

	public static final String CMD = "cmd";
	public static final String CMD_ARGV = "cmdArgv";
	public static final String EXEC_ADDR = "execAddr";
	public static final String HINT_PATH = "hintPath";
	public static final String ADD_PATH_TO_ARGV = "addPathToArgv";
	public static final String STREAM_STDOUT_KW = "streamStdout";
	public static final String DUMMY = "dummy";

	private final String commandToExecuteWithoutArguments;
	private final String argumentsToPassWithCommand;
	private final String executionHost;
	private final String absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn;
	private final boolean useBackwardCompatableInstruction;

	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("ExecCmd");
		sb.append("\n  commandToExecuteWithoutArguments:");
		sb.append(commandToExecuteWithoutArguments);
		sb.append("\n   argumentsToPassWithCommand:");
		sb.append(argumentsToPassWithCommand);
		sb.append("\n   executionHost:");
		sb.append(executionHost);
		sb.append("\n   absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn:");
		sb.append(absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn);
		sb.append("\n  useBackwardCompatableInstruction:\n");
		sb.append(useBackwardCompatableInstruction);
		return sb.toString();
	}

	/**
	 * Create an instance of the packing instruction to execute a remote command
	 * (script). This version is compatable with versions prior to iRODS 2.5.
	 * iRODS 2.5 added a dummy field for 64 bit alignment issues on some
	 * platform.
	 * 
	 * @param commandToExecuteWithoutArguments
	 *            <code>String</code> with the name of the command to execute.
	 *            Do not put arguments into this field.
	 * @param argumentsToPassWithCommand
	 *            <code>String</code> that is blank, or has the arguments to
	 *            send with the given command
	 * @return <code>ExcecCmd</code> instance.
	 * @throws JargonException
	 */
	public static final ExecCmd instanceWithCommandPriorTo25(
			final String commandToExecuteWithoutArguments,
			final String argumentsToPassWithCommand) throws JargonException {
		return new ExecCmd(STANDARD_EXEC_ENCAPSULATE_DATA_IN_RESPONSE_API_NBR,
				commandToExecuteWithoutArguments, argumentsToPassWithCommand,
				"", "", true);
	}

	/**
	 * Create an instance of the packing instruction to execute a remote command
	 * (script) using the newer API with 64 bit alignment for some platforms.
	 * 
	 * @param commandToExecuteWithoutArguments
	 *            <code>String</code> with the name of the command to execute.
	 *            Do not put arguments into this field.
	 * @param argumentsToPassWithCommand
	 *            <code>String</code> that is blank, or has the arguments to
	 *            send with the given command
	 * @return <code>ExcecCmd</code> instance.
	 * @throws JargonException
	 */
	public static final ExecCmd instanceWithCommandPost25(
			final String commandToExecuteWithoutArguments,
			final String argumentsToPassWithCommand) throws JargonException {
		return new ExecCmd(EXEC_AND_USE_ENHANCED_STREAM,
				commandToExecuteWithoutArguments, argumentsToPassWithCommand,
				"", "", false);
	}

	/**
	 * Create an instance of the packing instruction to execute a remote command
	 * (script). This initializer indicates to iRODS that large result buffers
	 * should be transmitted via a stream. This uses an enhanced API post iRODS
	 * 2.4.1 and is not supported in prior releases.
	 * 
	 * @param commandToExecuteWithoutArguments
	 *            <code>String</code> with the name of the command to execute.
	 *            Do not put arguments into this field.
	 * @param argumentsToPassWithCommand
	 *            <code>String</code> that is blank, or has the arguments to
	 *            send with the given command
	 * @return <code>ExcecCmd</code> instance.
	 * @throws JargonException
	 */
	public static final ExecCmd instanceWithCommandAllowingStreamingForLargeResultsPost25(
			final String commandToExecuteWithoutArguments,
			final String argumentsToPassWithCommand) throws JargonException {
		return new ExecCmd(EXEC_AND_USE_ENHANCED_STREAM,
				commandToExecuteWithoutArguments, argumentsToPassWithCommand,
				"", "", false);
	}

	/**
	 * Create an instance of the packing instruction to execute a remote command
	 * (script) This version is compatable with versions prior to iRODS 2.5.
	 * iRODS 2.5 added a dummy field for 64 bit alignment issues on some
	 * platform.
	 * 
	 * @param commandToExecuteWithoutArguments
	 *            <code>String</code> with the name of the command to execute.
	 *            Do not put arguments into this field.
	 * @param argumentsToPassWithCommand
	 *            <code>String</code> that is blank, or has the arguments to
	 *            send with the given command
	 * @param executionHost
	 *            <code>String</code> that can optionally point to the host on
	 *            which the command should be executed. Blank if not used.
	 * @param absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn
	 *            <code>String</code> that can optionally give an iRODS absolute
	 *            path. This is used within iRODS to find the host upon which
	 *            the file is located, and that host can be used to execute the
	 *            given command.
	 * @throws JargonException
	 */
	public static final ExecCmd instanceWithHostAndArgumentsToPassParametersPriorTo25(
			final String commandToExecuteWithoutArguments,
			final String argumentsToPassWithCommand,
			final String executionHost,
			final String absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn)
			throws JargonException {
		return new ExecCmd(STANDARD_EXEC_ENCAPSULATE_DATA_IN_RESPONSE_API_NBR,
				commandToExecuteWithoutArguments, argumentsToPassWithCommand,
				executionHost,
				absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn,
				true);
	}

	/**
	 * Create an instance of the packing instruction to execute a remote command
	 * (script) using the newer API with 64 bit alignment for some platforms.
	 * 
	 * @param commandToExecuteWithoutArguments
	 *            <code>String</code> with the name of the command to execute.
	 *            Do not put arguments into this field.
	 * @param argumentsToPassWithCommand
	 *            <code>String</code> that is blank, or has the arguments to
	 *            send with the given command
	 * @param executionHost
	 *            <code>String</code> that can optionally point to the host on
	 *            which the command should be executed. Blank if not used.
	 * @param absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn
	 *            <code>String</code> that can optionally give an iRODS absolute
	 *            path. This is used within iRODS to find the host upon which
	 *            the file is located, and that host can be used to execute the
	 *            given command.
	 * @throws JargonException
	 */
	public static final ExecCmd instanceWithHostAndArgumentsToPassParametersPost25(
			final String commandToExecuteWithoutArguments,
			final String argumentsToPassWithCommand,
			final String executionHost,
			final String absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn)
			throws JargonException {
		return new ExecCmd(EXEC_AND_USE_ENHANCED_STREAM,
				commandToExecuteWithoutArguments, argumentsToPassWithCommand,
				executionHost,
				absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn,
				false);
	}

	/**
	 * Create an instance of the packing instruction to execute a remote command
	 * (script). This initializer indicates to iRODS that large result buffers
	 * should be transmitted via a stream. This uses an API that only works with
	 * iRODS releases after 2.5
	 * 
	 * @param commandToExecuteWithoutArguments
	 *            <code>String</code> with the name of the command to execute.
	 *            Do not put arguments into this field.
	 * @param argumentsToPassWithCommand
	 *            <code>String</code> that is blank, or has the arguments to
	 *            send with the given command
	 * @param executionHost
	 *            <code>String</code> that can optionally point to the host on
	 *            which the command should be executed. Blank if not used.
	 * @param absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn
	 *            <code>String</code> that can optionally give an iRODS absolute
	 *            path. This is used within iRODS to find the host upon which
	 *            the file is located, and that host can be used to execute the
	 *            given command.
	 * @throws JargonException
	 */
	public static final ExecCmd instanceWithHostAndArgumentsToPassParametersAllowingStreamingForLargeResultsPost25(
			final String commandToExecuteWithoutArguments,
			final String argumentsToPassWithCommand,
			final String executionHost,
			final String absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn)
			throws JargonException {
		return new ExecCmd(EXEC_AND_USE_ENHANCED_STREAM,
				commandToExecuteWithoutArguments, argumentsToPassWithCommand,
				executionHost,
				absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn,
				false);
	}

	/**
	 * Constructor for a remote execution service packing instruction call.
	 * 
	 * @param apiNumber
	 *            <code>int</code> with the api number to use with this call.
	 * @param commandToExecuteWithoutArguments
	 *            <code>String</code> with the name of the command to execute.
	 *            Do not put arguments into this field.
	 * @param argumentsToPassWithCommand
	 *            <code>String</code> that is blank, or has the arguments to
	 *            send with the given command
	 * @param executionHost
	 *            <code>String</code> that can optionally point to the host on
	 *            which the command should be executed. Blank if not used.
	 * @param absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn
	 *            <code>String</code> that can optionally give an iRODS absolute
	 *            path. This is used within iRODS to find the host upon which
	 *            the file is located, and that host can be used to execute the
	 *            given command.
	 * @param useBackwardCompatableInstruction
	 *            <code>boolean</code> that indicates that the older version
	 *            (2.4.1 and prior) should be used. Otherwise, the newer API
	 *            with fixes for streaming and 64 bit alignment issues will be
	 *            used.
	 * @throws JargonException
	 */
	private ExecCmd(
			final int apiNumber,
			final String commandToExecuteWithoutArguments,
			final String argumentsToPassWithCommand,
			final String executionHost,
			final String absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn,
			final boolean useBackwardCompatableInstruction)
			throws JargonException {

		super();

		if (commandToExecuteWithoutArguments == null
				|| commandToExecuteWithoutArguments.length() == 0) {
			throw new JargonException("null commandToExecuteWithoutArguments");
		}

		if (argumentsToPassWithCommand == null) {
			throw new JargonException(
					"null argumentsToPassWithCommand, set to blank if not used");
		}

		if (executionHost == null) {
			throw new JargonException(
					"null executionHost, set to blank if not used");
		}

		if (absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn == null) {
			throw new JargonException(
					"null absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn, set to blank if not used");
		}

		if (getApiNumber() == STANDARD_EXEC_ENCAPSULATE_DATA_IN_RESPONSE_API_NBR
				&& useBackwardCompatableInstruction) {
			throw new JargonException(
					"cannot stream binary data using the older instruction, the parameters are in conflict");
		}

		this.commandToExecuteWithoutArguments = commandToExecuteWithoutArguments;
		this.argumentsToPassWithCommand = argumentsToPassWithCommand;
		this.executionHost = executionHost;
		this.absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn = absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn;
		this.useBackwardCompatableInstruction = useBackwardCompatableInstruction;
		this.setApiNumber(apiNumber);

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * org.irods.jargon.core.packinstr.AbstractIRODSPackingInstruction#getTagValue
	 * ()
	 */
	@Override
	public Tag getTagValue() throws JargonException {

		String piTagToUse;

		piTagToUse = PI_TAG;

		Tag message = new Tag(
				piTagToUse,
				new Tag[] {
						new Tag(CMD, commandToExecuteWithoutArguments),
						new Tag(CMD_ARGV, argumentsToPassWithCommand),
						new Tag(EXEC_ADDR, executionHost),
						new Tag(HINT_PATH,
								absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn),
						new Tag(ADD_PATH_TO_ARGV, 0) });

		if (!useBackwardCompatableInstruction) {
			// a dummy tag is in the pi for 64 bit alignment issues starting
			// post 2.4.1
			message.addTag(new Tag(DUMMY, 0));
		}

		if (!useBackwardCompatableInstruction) {
			message.addTag(Tag.createKeyValueTag(STREAM_STDOUT_KW, ""));
		} else {
			message.addTag(Tag.createKeyValueTag(null));
		}

		return message;
	}

	public String getCommandToExecuteWithoutArguments() {
		return commandToExecuteWithoutArguments;
	}

	public String getArgumentsToPassWithCommand() {
		return argumentsToPassWithCommand;
	}

	public String getExecutionHost() {
		return executionHost;
	}

	public String getAbsolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn() {
		return absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn;
	}

}
