package main;

import voter.Voter;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class Bajtocja {
	private List<District> districts;
	private Party[] parties;
	private int[][] possibleActions;
	
	
	public List<District> getDistricts() {
		return this.districts;
	}
	
	public Party[] getParties() {
		return this.parties;
	}
	
	public int getNumOfParties() {
		return this.parties.length;
	}
	
	public int getNumOfDistricts() {
		return this.districts.size();
	}
	
	public int getNumOfPossibleActions() {
		return this.possibleActions.length;
	}
	
	public int getNumOfAttributes() {
		return this.possibleActions[0].length; //zakładam, że będzie choć jedna akcja
	}
	
	/* Zwraca liczbę kandydatów we wszystkich okręgach. */
	public int getNumOfCandidates() {
		int ret = 0;
		for(District district: this.districts) {
			ret += district.getNumOfMandates() * this.parties.length;
		}
		return ret;
	}
	
	/* Zwraca liczbę głosujących we wszystkich okręgach. */
	public int getNumOfVoters() {
		int ret = 0;
		for(District district: this.districts) {
			ret += district.getNumOfVoters();
		}
		return ret;
	}
	
	/* Na podstawie numeru okręgu, nazwy partii i miejsca na liście ustala i zwraca
	kandydata.
	 */
	public Candidate getCandidate(int numOfDistrict, String partyName, int candidatePosition) {
		Party party = getParty(partyName);
		return districts.get(numOfDistrict - 1).getCandidate(party, candidatePosition);
	}
	
	/* Zwraca partię o danej nazwie. */
	public Party getParty(String partyName) {
		for(Party party: parties) {
			if(party.getName().equals(partyName)) {
				return party;
			}
		}
		return null;
	}
	
	
	/* Tworzy i inicjalizuje listę okręgów nowymi okręgami, nadając im numery. */
	public void setNumOfDistricts(int numOfDistricts) {
		this.districts = new ArrayList<>();
		for(int i = 0; i < numOfDistricts; ++i) {
			this.districts.add(new District(i + 1));
		}
	}
	
	/* Tworzy i inicjalizuje tablicę partii. */
	public void setNumOfParties(int numOfParties) {
		this.parties = new Party[numOfParties];
		for(int i = 0; i < numOfParties; ++i) {
			this.parties[i] = new Party();
		}
	}
	
	/* Tworzy nową tablicę możliwych akcji. */
	public void setNumOfPossibleActions(int possibleActions, int attributes) {
		this.possibleActions = new int[possibleActions][attributes];
	}
	
	/* Ustawia po kolei nazwy wszystkim partiom. */
	public void setPartiesNames(String[] partiesNames) {
		for(int i = 0; i < parties.length; ++i) {
			this.parties[i].setName(partiesNames[i]);
		}
	}
	
	/* Ustawia po kolei budżety wszystkim partiom. */
	public void setPartiesBudgets(int[] partiesBudgets) {
		for(int i = 0; i < parties.length; ++i) {
			this.parties[i].setBudget(partiesBudgets[i]);
		}
	}
	
	/* Ustawia po kolei strategie wszystkim partiom. */
	public void setPartiesStrategies(String[] partiesStrategies) {
		for(int i = 0; i < parties.length; ++i) {
			this.parties[i].setStrategy(partiesStrategies[i].charAt(0));
		}
	}
	
	/* Ustawia liczbę głosujących we wszystkich okręgach. */
	public void setVotersInDistricts(int[] numOfVotersInDistricts) {
		int i = 0;
		for(District district: this.districts) {
			district.setNumOfVoters(numOfVotersInDistricts[i]);
			++i;
		}
	}
	
	/* Zapisuje akcje, mozliwe do wykonania przez partie, w atrybucie. */
	public void setPossibleActions(int[][] possibleActions) {
		this.possibleActions = possibleActions;
	}
	
	
	/* Oznacza okręgi, które mają być połączone. */
	public void markDistrictsToMerge(int district1, int district2) {
		this.districts.get(district1 - 1).markToMerge();
		this.districts.get(district2 - 1).markToMerge();
	}
	
	/* Dodaje pojedynczego kandydata do okręgu. */
	public void addCandidateToDistrict(Candidate candidate) {
		int numOfDistrict = candidate.getNumOfDistrict();
		String partyName = candidate.getPartyName();
		for(Party party: this.parties) {
			if(party.getName().equals(partyName)) {
				this.districts.get(numOfDistrict - 1).addCandidate(candidate, party);
			}
		}
	}
	
	/* Dodaje pojedynczego wyborcę do okręgu. */
	public void addVoterToDistrict(Voter voter) {
		int numOfDistrict = voter.getNumOfDistrict();
		this.districts.get(numOfDistrict - 1).addVoter(voter);
	}
	
	/* Łączy wcześniej oznaczone okręgi. */
	public void mergeDistricts() {
		Iterator<District> iter = districts.iterator();
		District first = null, second;
		while(iter.hasNext()) {
			second = iter.next();
			if(first != null && first.markedToMerge() && second.markedToMerge()) {
				first.mergeWith(second);
				iter.remove();
				first = null;
			}
			else {
				first = second;
			}
		}
	}
	
	public void runCampaign() {
		for(Party party: this.parties) {
			party.runCampaign(this.possibleActions, this.districts);
		}
	}
}
