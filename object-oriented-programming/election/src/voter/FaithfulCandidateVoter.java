package voter;

import main.Candidate;
import main.Party;

import java.util.List;
import java.util.Map;

/* Żelazny elektorat kandydata. */
public class FaithfulCandidateVoter extends Voter {
	protected Candidate candidate;
	
	public FaithfulCandidateVoter(String name, String surname, int numOfDistrict,
								  Candidate candidate) {
		super(name, surname, numOfDistrict);
		this.candidate = candidate;
	}
	
	
	@Override
	public void vote(Map<Party, List<Candidate>> ticket) {
		this.chosenCandidate = this.candidate;
		this.chosenCandidate.addVote();
	}
}
