package main;

import voter.*;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;
import java.util.regex.Pattern;

public class Input {
	
	/* Czyta kolejne linijki z wejścia zgodnie ze specyfikacją. Dane po przeczytaniu są
	* zapisywane do odpowiednich klas. Wyrzuca wyjątek, gdy podany plik z danymi
	* wejściowymi nie istnieje. */
	public void readInput(Bajtocja bajtocja, File file) throws FileNotFoundException {
		Scanner scan = new Scanner(file);
		readFirstLine(bajtocja, scan);
		readSecondLine(bajtocja, scan);
		readThirdLine(bajtocja, scan);
		readFourthLine(bajtocja, scan);
		readFifthLine(bajtocja, scan);
		readSixthLine(bajtocja, scan);
		readCandidatesData(bajtocja, scan);
		readVotersData(bajtocja, scan);
		readPossibleActions(bajtocja, scan);
		scan.close();
	}
	
	/* Czyta liczbę okręgów, partii, możliwych akcji i atrybutów kandydatów. */
	public void readFirstLine(Bajtocja bajtocja, Scanner scan) {
		int numOfDistricts, numOfParties, numOfPossibleActions, numOfAttributes;
		
		numOfDistricts = scan.nextInt();
		numOfParties = scan.nextInt();
		numOfPossibleActions = scan.nextInt();
		numOfAttributes = scan.nextInt();
		
		bajtocja.setNumOfDistricts(numOfDistricts);
		bajtocja.setNumOfParties(numOfParties);
		bajtocja.setNumOfPossibleActions(numOfPossibleActions, numOfAttributes);
	}
	
	/* Czyta, które okręgi należy połączyć. */
	public void readSecondLine(Bajtocja bajtocja, Scanner scan) {
		int numOfMergedDistricts = scan.nextInt();
		int district1, district2;
		Pattern pattern = scan.delimiter();
		while(scan.hasNext("\\(\\d,\\d\\)")) {
			scan.useDelimiter("\\s\\(|,|\\)\\s\\(|\\)");
			district1 = scan.nextInt();
			district2 = scan.nextInt();
			bajtocja.markDistrictsToMerge(district1, district2);
			scan.useDelimiter(pattern);
			scan.next("\\)");
		}
		scan.nextLine();
	}
	
	/* Czyta nazwy partii. */
	public void readThirdLine(Bajtocja bajtocja, Scanner scan) {
		String line = scan.nextLine();
		String[] partiesNames = line.split(" ");
		bajtocja.setPartiesNames(partiesNames);
	}
	
	/* Czyta budżety partii. */
	public void readFourthLine(Bajtocja bajtocja, Scanner scan) {
		int[] budgets = new int[bajtocja.getNumOfParties()];
		int i = 0;
		while(scan.hasNextInt()) {
			budgets[i] = scan.nextInt();
			++i;
		}
		scan.nextLine();
		
		bajtocja.setPartiesBudgets(budgets);
	}
	
	/* Czyta strategie partii, zadane odpowiednimi literami. */
	public void readFifthLine(Bajtocja bajtocja, Scanner scan) {
		String line = scan.nextLine();
		String[] strategies = line.split(" ");
		bajtocja.setPartiesStrategies(strategies);
	}
	
	/* Czyta liczby głosujących w kolejnych okręgach. */
	public void readSixthLine(Bajtocja bajtocja, Scanner scan) {
		int[] numOfVotersInDistricts = new int[bajtocja.getNumOfDistricts()];
		int i = 0;
		while(scan.hasNextInt()) {
			numOfVotersInDistricts[i] = scan.nextInt();
			++i;
		}
		scan.nextLine();
		
		bajtocja.setVotersInDistricts(numOfVotersInDistricts);
	}
	
	/* Czyta kolejne linijki z danymi kandydatów. */
	public void readCandidatesData(Bajtocja bajtocja, Scanner scan) {
		int numOfCandidates = bajtocja.getNumOfCandidates();
		for(int i = 0; i < numOfCandidates; ++i) {
			String line = scan.nextLine();
			String[] candidateData = line.split(" ");
			
			String name, surname, partyName;
			int district, positionOnList;
			int[] attributes = new int[candidateData.length - 5];
			/* Pierwszych pięć napisów zapiszemy do wyżej zadeklarowanych zmiennych,
			pozostałe to wartości cech kandydata. */
			
			name = candidateData[0];
			surname = candidateData[1];
			district = Integer.parseInt(candidateData[2]);
			partyName = candidateData[3];
			positionOnList = Integer.parseInt(candidateData[4]);
			
			for(int j = 0, k = 5;  j < attributes.length;  ++j, ++k) {
				attributes[j] = Integer.parseInt(candidateData[k]);
			}
			
			Candidate candidate = new Candidate(name, surname, partyName, district,
					positionOnList, attributes);
			bajtocja.addCandidateToDistrict(candidate);
		}
	}
	
	/* Czyta kolejne linijki z danymi głosujących. */
	public void readVotersData(Bajtocja bajtocja, Scanner scan) {
		int numOfVoters = bajtocja.getNumOfVoters();
		for(int i = 0; i < numOfVoters; ++i) {
			String line = scan.nextLine();
			String[] voterData = line.split(" ");
			
			String name, surname;
			int numOfDistrict, type;
			
			name = voterData[0];
			surname = voterData[1];
			numOfDistrict = Integer.parseInt(voterData[2]);
			type = Integer.parseInt(voterData[3]);
			
			Voter voter = createVoter(name, surname, numOfDistrict, type, voterData, bajtocja);
			
			bajtocja.addVoterToDistrict(voter);
		}
	}
	
	/* Tworzy nowego głosującego odpowiedniego typu, na podstawie danych otrzymanych
	jako argumenty, a wcześniej przeczytanych z wejścia.
	 */
	private Voter createVoter(String name, String surname, int numOfDistrict, int type,
							  String[] voterData, Bajtocja bajtocja) {
		Voter voter;
		if(type == 1 || type == 2) {
			String partyName = voterData[4];
			
			if(type == 1) {
				voter = new FaithfulPartyVoter(name, surname, numOfDistrict,
						bajtocja.getParty(partyName));
			}
			else { //type == 2
				int candidatePositionOnList = Integer.parseInt(voterData[5]);
				voter = new FaithfulCandidateVoter(name, surname, numOfDistrict,
						bajtocja.getCandidate(numOfDistrict, partyName, candidatePositionOnList));
			}
		}
		else if(type == 5 || type == 8) {
			int[] attributesWeights = new int[bajtocja.getNumOfAttributes()];
			for(int j = 0, k = 4;  j < attributesWeights.length;  ++j, ++k) {
				attributesWeights[j] = Integer.parseInt(voterData[k]);
			}
			
			if(type == 5) {
				voter = new VersatileVoter(name, surname, numOfDistrict, attributesWeights);
			}
			else { //type == 8
				String partyName = voterData[voterData.length - 1];
				voter = new VersatileVoterParty(name, surname, numOfDistrict,
						attributesWeights, bajtocja.getParty(partyName));
			}
		}
		else { //(type == 3 || type == 4 || type == 6 || type == 7)
			int numOfAttribute = Integer.parseInt(voterData[4]);
			
			if(type == 3) {
				voter = new MiniAttributeVoter(name, surname, numOfDistrict, numOfAttribute);
			}
			else if(type == 4) {
				voter = new MaxAttributeVoter(name, surname, numOfDistrict, numOfAttribute);
			}
			else { //(type == 6 || type == 7)
				String partyName = voterData[5];
				if(type == 6) {
					voter = new MiniAttributeVoterParty(name, surname, numOfDistrict,
							numOfAttribute, bajtocja.getParty(partyName));
				}
				else { //type == 7
					voter = new MaxAttributeVoterParty(name, surname, numOfDistrict,
							numOfAttribute, bajtocja.getParty(partyName));
				}
			}
		}
		return voter;
	}
	
	/* Czyta kolejne linijki zawierające akcje, będące wektorami zmian wag cech
	u wyborców.
	 */
	public void readPossibleActions(Bajtocja bajtocja, Scanner scan) {
		int numOfActions = bajtocja.getNumOfPossibleActions();
		int numOfAttributes = bajtocja.getNumOfAttributes();
		int[][] possibleActions = new int[numOfActions][numOfAttributes];
		for(int i = 0; i < numOfActions; ++i) {
			for(int j = 0; j < numOfAttributes; ++j) {
				possibleActions[i][j] = scan.nextInt();
			}
		}
		scan.nextLine();
		
		bajtocja.setPossibleActions(possibleActions);
	}
}
