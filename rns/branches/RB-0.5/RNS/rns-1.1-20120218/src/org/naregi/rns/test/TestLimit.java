package org.naregi.rns.test;

import java.util.List;

import org.apache.axis.message.MessageElement;
import org.naregi.rns.client.RNSClient;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSError;
import org.naregi.rns.util.RNSUtil;

/**
 * Check limitation of Metadata size.
 */
public class TestLimit {

	private static String str = null;

	private static String str100KB() {
		if (str != null) {
			return str;
		}
		StringBuilder sb = new StringBuilder();
		for (int i = 0; i < 1000; i++) {
			/* 100byte */
			sb.append("0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
		}
		str = sb.toString();
		return str;
	}

	private static String generateBigXML(int n) throws Exception {
		StringBuilder sb = new StringBuilder();
		sb.append("<test>");

		/* 100KB * n */
		for (int i = 0; i < n; i++) {
			sb.append("<tag" + i + ">");
			sb.append(str100KB());
			sb.append("</tag" + i + ">");
		}
		sb.append("</test>");

		return sb.toString();
	}

	public static void main(String[] args) {
		RNSClientHome home = new RNSClientHome();
		boolean usage = true;
		try {
			home.setCustomUsage("[min(*100KB) [max(*100KB)]]");
			List<String> al = home.parseArgs(args, 0, 2);
			RNSClient rnsc = home.getRNSClient();

			String path = "/_TEST_LIMIT";
			int min = 1; /* 100KB */
			if (al.size() >= 1) {
				min = Integer.parseInt(al.get(0));
			}
			int max = 1000; /* 100MB */
			if (al.size() >= 2) {
				max = Integer.parseInt(al.get(1));
			}
			usage = false;

			try {
				rnsc.mkdir(path);
			} catch (RNSError e) {
				/* ignore */
			}

			try {
				for (int i = min; i <= max; i++) {
					String xml = generateBigXML(i);
					System.out.print("XML size = " + xml.length());
					MessageElement[] mes = RNSUtil.toMessageElements(xml);

					long t1 = System.nanoTime();
					rnsc.setMetadata(path, mes);
					long t2 = System.nanoTime();
					System.out.println(" (" + ((t2 - t1) / 1000000) + " ms)");
				}
				rnsc.rmdir(path);
			} catch (Exception e2) {
				try {
					rnsc.rmdir(path);
				} catch (Exception e3) {
				}
				throw e2;
			}
		} catch (Exception e) {
			System.out.println();
			home.printError(e, System.err);
			if (usage) {
				home.printUsage(System.out);
			}
			System.exit(1);
			return;
		}
	}
}
