/* Copyright (c) 2014-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package org.simgrid;

import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;

/** Helper class loading the native functions of SimGrid that we use for downcalls
 *
 * Almost all org.simgrid.msg.* classes contain a static bloc (thus executed when the class is loaded)
 * containing a call to this.
 */
public final class NativeLib {
	private static boolean isNativeInited = false;
	private static Path tempDir = null; // where the embeeded libraries are unpacked before loading them

	/** A static-only "class" don't need no constructor */
	private NativeLib() {
		throw new IllegalAccessError("Utility class");
	}

	/** Hidden debug main() function
	 *
	 * It is not the Main-Class defined in src/bindings/java/MANIFEST.in (org.simgrid.msg.Msg is),
	 * so it won't get executed by default. But that's helpful to debug linkage errors, if you
	 * know that it exists. It's used by cmake during the configure, to inform the user.
	 */
	public static void main(String[] args) {
		System.out.println("This jarfile searches the native code under: " +getPath());
	}
	
	/** Main function loading all the native classes that we need */
	public static void nativeInit() {
		if (isNativeInited)
			return;

		if (System.getProperty("os.name").toLowerCase().startsWith("win"))
			NativeLib.nativeInit("winpthread-1");

		NativeLib.nativeInit("simgrid");
		NativeLib.nativeInit("simgrid-java");
		isNativeInited = true;
	}

	/** Helper function trying to load one requested library */
	public static void nativeInit(String name) {
		Throwable cause = null;
		try {
			/* Prefer the version of the library bundled into the jar file and use it */
			if (loadLibAsStream(name))
				return;
		} catch (UnsatisfiedLinkError|SecurityException|IOException e) {
			cause = e;
		}
		
		/* If not found, try to see if we can find a version on disk */
		try {
			System.loadLibrary(name);
			return;
		} catch (UnsatisfiedLinkError systemException) { /* don't care */ }
		
		System.err.println("\nCannot load the bindings to the "+name+" library in path "+getPath()+" and no usable SimGrid installation found on disk.");
		if (cause != null) {
			if (cause.getMessage().matches(".*libcgraph.so.*"))
				System.err.println("HINT: Try to install the libcgraph package (sudo apt-get install libcgraph).");
			else if (cause.getMessage().matches(".*libboost_context.so.*"))
				System.err.println("HINT: Try to install the boost-context package (sudo apt-get install libboost-context-dev).");
			else
				System.err.println("Try to install the missing dependencies, if any. Read carefully the following error message.");

			System.err.println();
			cause.printStackTrace();
		} else {
			System.err.println("This jar file does not seem to fit your system, and no usable SimGrid installation found on disk for "+name+".");
		}
		System.exit(1);
	}

	/** Try to extract the library from the jarfile before loading it */
	private static boolean loadLibAsStream (String name) throws IOException, UnsatisfiedLinkError {
		String path = NativeLib.getPath();
		
		// We must write the lib onto the disk before loading it -- stupid operating systems
		if (tempDir == null) {
			tempDir = Files.createTempDirectory("simgrid-java-");
			// don't leak the files on disk, but remove it on JVM shutdown
			Runtime.getRuntime().addShutdownHook(new Thread(new FileCleaner(tempDir.toFile())));
		}
		
		/* For each possible filename of the given library on all possible OSes, try it */
		for (String filename : new String[]
		   { name,
		     "lib"+name+".so",               /* linux */
		     name+".dll", "lib"+name+".dll", /* windows (pure and mingw) */
		     "lib"+name+".dylib"             /* mac osx */}) {
						
			File fileOut = new File(tempDir.toFile().getAbsolutePath() + File.separator + filename);
			boolean done = false;
			try ( // Try-with-resources. These stream will be autoclosed when needed.
				InputStream in = NativeLib.class.getClassLoader().getResourceAsStream(path+filename);
				OutputStream out = new FileOutputStream(fileOut);
			) {
				if (in != null) {
					/* copy the library in position */
					byte[] buffer = new byte[4096];
					int bytesRead;
					while ((bytesRead = in.read(buffer)) != -1)     // Read until EOF
						out.write(buffer, 0, bytesRead);
					done = true;
				}
			}
			if (done) {
				/* load that library */
				System.load(fileOut.getAbsolutePath());

				/* It loaded! we're good */
				return true;
			}
		}
		
		/* No suitable name found */
		return false;
	}

	/** Find where to search for the library in the jar -- keep it aligned with where cmake puts it! */
	private static String getPath() {
		// Inspiration: https://github.com/xerial/snappy-java/blob/develop/src/main/java/org/xerial/snappy/OSInfo.java
		String prefix = "NATIVE";
		String os = System.getProperty("os.name");
		String arch = System.getProperty("os.arch");

		if (arch.matches("^i[3-6]86$"))
			arch = "x86";
		else if ("x86_64".equalsIgnoreCase(arch) || "AMD64".equalsIgnoreCase(arch))
			arch = "amd64";

		if (os.toLowerCase().startsWith("win")) {
			os = "Windows";
		} else if (os.contains("OS X")) {
			os = "Darwin";
		}
		os = os.replace(' ', '_');
		arch = arch.replace(' ', '_');

		return prefix + "/" + os + "/" + arch + "/";
	}
	
	/** A hackish mechanism used to remove the file containing our library when the JVM shuts down */
	private static class FileCleaner implements Runnable {
		private File dir;
		public FileCleaner(File dir) {
			this.dir = dir;
		}
		@Override
		public void run() {
			try {
				for (File f : dir.listFiles())
					if (! f.delete() && !f.getAbsolutePath().contains("appveyor")) // Be silent on AppVeyor to not break the tests. Ugly trick :)
						System.out.println("Unable to clean temporary file "+f.getAbsolutePath()+" during shutdown.");
				if (! dir.delete() && !dir.getAbsolutePath().contains("appveyor") )
					System.out.println("Unable to clean temporary file "+dir.getAbsolutePath()+" during shutdown.");			    	
			} catch(Exception e) {
				System.out.println("Error while cleaning temporary file "+dir.getAbsolutePath()+" during shutdown: "+e.getCause());
				e.printStackTrace();
			}
		}
	}
}
