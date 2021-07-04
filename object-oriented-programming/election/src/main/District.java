package main;

import voter.Voter;

import java.util.*;

public class District {
	private int number;
	private int numOfVoters;
	private int numOfMandates;
	private List<Voter> voters;
	private Map<Party, List<Candidate>> ticket;
	private boolean merge;
	private Map<String, Integer> votesPerParty;
	private Map<String, Integer> mandatesPerParty;
	
	
	public District(int number) {
		this.number = number;
		this.voters = new ArrayList<>();
		this.ticket = new LinkedHashMap<>();
		this.merge = false;
		this.votesPerParty = new LinkedHashMap<>();
	}
	
	public int getNumber() {
		return this.number;
	}
	
	public int getNumOfVoters() {
		return this.numOfVoters;
	}
	
	public int getNumOfMandates() {
		return this.numOfMandates;
	}
	
	public List<Voter> getVoters() {
		return this.voters;
	}
	
	public List<Candidate> getCandidates(Party party) {
		return ticket.get(party);
	}
	
	public Map<Party, List<Candidate>> getTicket() {
		return this.ticket;
	}
	
	public Map<String, Integer> getVotesPerParty() {
		return votesPerParty;
	}
	
	public Map<String, Integer> getMandates() {
		return this.mandatesPerParty;
	}
	
	/* Na podstawie partii i miejsca na liście ustala i zwraca kandydata. */
	public Candidate getCandidate(Party party, int candidatePosition) {
		return this.ticket.get(party).get(candidatePosition - 1);
	}
	
	
	public void setNumOfVoters(int numOfVoters) {
		this.numOfVoters = numOfVoters;
		this.numOfMandates = numOfVoters / 10;
	}
	
	public void setMandatesPerParty(Map<String, Integer> mandatesPerParty) {
		this.mandatesPerParty = new LinkedHashMap<>(mandatesPerParty);
	}
	
	
	/* Dodaje kandydata na listę partii. */
	public void addCandidate(Candidate candidate, Party party) {
		if(ticket.isEmpty() || !ticket.containsKey(party)) {
			List<Candidate> list = new ArrayList<>();
			list.add(candidate);
			ticket.put(party, list);
		}
		else {
			ticket.get(party).add(candidate);
		}
	}
	
	/* Dodaje głosującego do listy wyborcow. */
	public void addVoter(Voter voter) {
		this.voters.add(voter);
	}
	
	/* Oznacza okrąg to późniejszego połączenie. */
	public void markToMerge() {
		this.merge = true;
	}
	
	/* Zwraca inforamację czy okręg powinien zostać połączony. */
	public boolean markedToMerge() {
		return this.merge;
	}
	
	/* Łączy dwa okręgi. Z okręgu przekazanego jako argument przepisywani do tego okręgu
	są wszyscy kandydaci i wyborcy. Kandydaci mają zmieniany numer okręgu i miejsce na
	liście. Liczba mandatów i wyborców jest aktualizowana o tę z drugiego okręgu.
	 */
	public void mergeWith(District second) {
		int listLength = this.numOfMandates;
		
		this.numOfVoters += second.getNumOfVoters();
		this.numOfMandates += second.getNumOfMandates();
		
		List<Voter> voters2 = second.getVoters();
		for(Voter voter: voters2) {
			voter.setNumOfDistrict(this.number);
		}
		this.voters.addAll(voters2);
		
		Map<Party, List<Candidate>> ticket2 = second.getTicket();
		for(Map.Entry<Party, List<Candidate>> entry: ticket2.entrySet()) {
			Party party = entry.getKey();
			List<Candidate> listOfCandidates = entry.getValue();
			for(Candidate candidate: listOfCandidates) {
				candidate.setNumOfDistrict(this.number);
				candidate.setPositionOnList(listLength + candidate.getPositionOnList());
			}
			this.ticket.get(party).addAll(listOfCandidates);
		}
		
		this.merge = false;
	}
	
	/* Pozwala oddać głos kaźdemu wyborcy. */
	public void conductVoting() {
		for(Voter voter: voters) {
			voter.vote(ticket);
		}
	}
	
	/* Przelicza głosy dla wszystkich partii w okręgu. */
	public void countVotes() {
		for(Map.Entry<Party, List<Candidate>> entry: ticket.entrySet()) {
			String partyName = entry.getKey().getName();
			List<Candidate> listOfCandidates = entry.getValue();
			votesPerParty.put(partyName, 0);
			
			for(Candidate candidate: listOfCandidates) {
				votesPerParty.put(partyName, votesPerParty.get(partyName) + candidate.getReceivedVotes());
			}
		}
	}
	
	/* Pokazuje wyniki, wypisując numer okręgu, informacje o głosowaniu dla wszystkich
	wyborców i kandydatów, a na koniec podsumowanie - podział mandatów dla partii w
	okręgu.
	 */
	public void showResults() {
		System.out.println(this.number);
		
		for(Voter voter: this.voters) {
			System.out.println(voter.toString());
		}
		
		for(Map.Entry<Party, List<Candidate>> entry: ticket.entrySet()) {
			List<Candidate> listOfCandidates = entry.getValue();
			
			for(Candidate candidate: listOfCandidates) {
				System.out.println(candidate.toString());
			}
		}
		
		for(Map.Entry<String, Integer> entry: this.mandatesPerParty.entrySet()) {
			System.out.println(entry.getKey() + " " + entry.getValue());
		}
	}
}
